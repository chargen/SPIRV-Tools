// Copyright (c) 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "interface_cleanup_pass.h"

#include "ir_context.h"
#include "reflect.h"

namespace spvtools {
namespace opt {
namespace {
const uint32_t kTypePointerStorageClassInIdx = 0;
const uint32_t kEntryPointVariableIdx = 3;

bool HasExternalLinkage(analysis::DecorationManager* decoration_mgr,
                        uint32_t result_id) {
  bool external = false;
  decoration_mgr->ForEachDecoration(
      result_id, SpvDecorationLinkageAttributes,
      [&external](const ir::Instruction& linkage_instruction) {
        uint32_t last_operand = linkage_instruction.NumOperands() - 1;
        if (linkage_instruction.GetSingleWordOperand(last_operand) ==
            SpvLinkageTypeExport) {
          external = true;
        }
      });
  return external;
}

bool IsUsedExceptInEntryPoint(analysis::DefUseManager* def_use_mgr,
                              uint32_t result_id) {
  bool used = false;
  def_use_mgr->ForEachUser(result_id, [&used](ir::Instruction* user) {
    if (!ir::IsAnnotationInst(user->opcode()) && user->opcode() != SpvOpName &&
        user->opcode() != SpvOpEntryPoint) {
      used = true;
    }
  });
  return used;
}

SpvStorageClass GetStorageClass(analysis::DefUseManager* def_use_mgr,
                                uint32_t result_id) {
  ir::Instruction* def = def_use_mgr->GetDef(result_id);
  assert(def != nullptr);
  assert(def->opcode() == SpvOpVariable);

  ir::Instruction* type = def_use_mgr->GetDef(def->type_id());
  assert(type);
  assert(type->opcode() == SpvOpTypePointer);

  return SpvStorageClass(
      type->GetSingleWordInOperand(kTypePointerStorageClassInIdx));
}

SpvBuiltIn GetBuiltInDecoration(analysis::DecorationManager* decoration_mgr,
                                uint32_t result_id) {
  SpvBuiltIn result = SpvBuiltInMax;
  decoration_mgr->ForEachDecoration(
      result_id, SpvDecorationBuiltIn,
      [&result](const ir::Instruction& linkage_instruction) {
        uint32_t last_operand = linkage_instruction.NumOperands() - 1;
        result =
            SpvBuiltIn(linkage_instruction.GetSingleWordOperand(last_operand));
      });
  return result;
}

}  // namespace

Pass::Status InterfaceCleanupPass::Process(ir::IRContext* c) {
  InitializeProcessing(c);

  bool modified = false;

  for (auto& entrypoint : context()->module()->entry_points()) {
    assert(entrypoint.opcode() == SpvOpEntryPoint);

    for (uint32_t op = kEntryPointVariableIdx; op < entrypoint.NumOperands();) {
      uint32_t var = entrypoint.GetSingleWordInOperand(op);

      if (HasExternalLinkage(get_decoration_mgr(), var)) {
        ++op;
        continue;
      }

      if (IsUsedExceptInEntryPoint(get_def_use_mgr(), var)) {
        ++op;
        continue;
      }

      auto storage_class = GetStorageClass(get_def_use_mgr(), var);
      auto builtin = GetBuiltInDecoration(get_decoration_mgr(), var);

      // output variables have undefined values if shader doesn't write them
      // removing them might affect interface compatibility if the next stage
      // reads them (this produces undefined values but otherwise is valid?)
      if (storage_class != SpvStorageClassInput) {
        ++op;
        continue;
      }

      // presence of SampleId builtin affects semantics (forces sample shading)
      if (builtin == SpvBuiltInSampleId) {
        ++op;
        continue;
      }

      modified = true;

      entrypoint.RemoveOperand(op);

      context()->KillDef(var);
    }
  }

  return (modified ? Status::SuccessWithChange : Status::SuccessWithoutChange);
}
}  // namespace opt
}  // namespace spvtools
