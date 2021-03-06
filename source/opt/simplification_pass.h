// Copyright (c) 2018 Google LLC
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

#ifndef LIBSPIRV_OPT_SIMPLIFICATION_PASS_H_
#define LIBSPIRV_OPT_SIMPLIFICATION_PASS_H_

#include "function.h"
#include "ir_context.h"
#include "pass.h"

namespace spvtools {
namespace opt {

// See optimizer.hpp for documentation.
class SimplificationPass : public Pass {
 public:
  const char* name() const override { return "simplify-instructions"; }
  Status Process(ir::IRContext*) override;
  virtual ir::IRContext::Analysis GetPreservedAnalyses() override {
    return ir::IRContext::kAnalysisDefUse |
           ir::IRContext::kAnalysisInstrToBlockMapping |
           ir::IRContext::kAnalysisDecorations |
           ir::IRContext::kAnalysisCombinators | ir::IRContext::kAnalysisCFG |
           ir::IRContext::kAnalysisDominatorAnalysis |
           ir::IRContext::kAnalysisNameMap;
  }

 private:
  // Returns true if the module was changed.  The simplifier is called on every
  // instruction in |function| until nothing else in the function can be
  // simplified.
  bool SimplifyFunction(ir::Function* function);
};

}  // namespace opt
}  // namespace spvtools

#endif  // LIBSPIRV_OPT_SIMPLIFICATION_PASS_H_
