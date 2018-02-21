// Copyright (c) 2017 Valve Corporation
// Copyright (c) 2017 LunarG Inc.
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

#include "pass_fixture.h"
#include "pass_utils.h"

namespace {

using namespace spvtools;

using InterfaceCleanupTest = PassTest<::testing::Test>;

TEST_F(InterfaceCleanupTest, Basic) {
  std::string before = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %Out %Dead
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %Out "Out"
OpName %Dead "Dead"
OpDecorate %Out Location 0
OpDecorate %Dead Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%Out = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
%11 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%Dead = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %3
%5 = OpLabel
OpStore %Out %11
OpReturn
OpFunctionEnd
)";

  std::string after = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %Out
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %Out "Out"
OpDecorate %Out Location 0
%void = OpTypeVoid
%6 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%Out = OpVariable %_ptr_Output_v4float Output
%float_0 = OpConstant %float 0
%11 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%main = OpFunction %void None %6
%13 = OpLabel
OpStore %Out %11
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<opt::InterfaceCleanupPass>(before, after, true, true);
}

TEST_F(InterfaceCleanupTest, DeadAndUsed) {
  std::string before = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %Out %Dead %Used
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %Out "Out"
OpName %Used "Used"
OpName %Dead "Dead"
OpDecorate %Out Location 0
OpDecorate %Used Location 1
OpDecorate %Dead Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%Out = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%Used = OpVariable %_ptr_Input_v4float Input
%Dead = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %3
%5 = OpLabel
%12 = OpLoad %v4float %Used
OpStore %Out %12
OpReturn
OpFunctionEnd
)";

  std::string after = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %Out %Used
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpName %main "main"
OpName %Out "Out"
OpName %Used "Used"
OpDecorate %Out Location 0
OpDecorate %Used Location 1
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%Out = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%Used = OpVariable %_ptr_Input_v4float Input
%main = OpFunction %void None %7
%12 = OpLabel
%13 = OpLoad %v4float %Used
OpStore %Out %13
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<opt::InterfaceCleanupPass>(before, after, true, true);
}

TEST_F(InterfaceCleanupTest, DeadBuiltin) {
  std::string before = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %Dead %_entryPointOutput
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %Dead "Dead"
OpName %_entryPointOutput "@entryPointOutput"
OpDecorate %Dead BuiltIn FragCoord
OpDecorate %_entryPointOutput Location 0
%void = OpTypeVoid
%3 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%float_0 = OpConstant %float 0
%14 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%Dead = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %3
%5 = OpLabel
OpStore %_entryPointOutput %14
OpReturn
OpFunctionEnd
)";

  std::string after = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %_entryPointOutput
OpExecutionMode %main OriginUpperLeft
OpSource HLSL 500
OpName %main "main"
OpName %_entryPointOutput "@entryPointOutput"
OpDecorate %_entryPointOutput Location 0
%void = OpTypeVoid
%6 = OpTypeFunction %void
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%float_0 = OpConstant %float 0
%10 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
%main = OpFunction %void None %6
%13 = OpLabel
OpStore %_entryPointOutput %10
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<opt::InterfaceCleanupPass>(before, after, true, true);
}

}  // anonymous namespace
