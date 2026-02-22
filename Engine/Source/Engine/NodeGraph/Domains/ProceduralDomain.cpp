#include "NodeGraph/Domains/ProceduralDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/ProceduralNodes.h"

void ProceduralDomain::RegisterNodeTypes()
{
    // Math nodes
    AddNodeType(AddNode::GetStaticType(), "Add", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SubtractNode::GetStaticType(), "Subtract", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyNode::GetStaticType(), "Multiply", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(DivideNode::GetStaticType(), "Divide", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(LerpNode::GetStaticType(), "Lerp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ClampNode::GetStaticType(), "Clamp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(AbsNode::GetStaticType(), "Abs", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(DotProductNode::GetStaticType(), "Dot Product", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RemapValueNode::GetStaticType(), "Remap Value", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SinNode::GetStaticType(), "Sin", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));


    // Value nodes
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(IntConstantNode::GetStaticType(), "Integer", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(VectorConstantNode::GetStaticType(), "Vector", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(ColorConstantNode::GetStaticType(), "Color", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(TimeNode::GetStaticType(), "Time", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));

    // Conversion nodes - To String
    AddNodeType(FloatToStringNode::GetStaticType(), "Float To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(IntToStringNode::GetStaticType(), "Int To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(BoolToStringNode::GetStaticType(), "Bool To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(VectorToStringNode::GetStaticType(), "Vector To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(ColorToStringNode::GetStaticType(), "Color To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(NodeToStringNode::GetStaticType(), "Node To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(Node3DToStringNode::GetStaticType(), "Node3D To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));

    // Conversion nodes - From String
    AddNodeType(StringToFloatNode::GetStaticType(), "String To Float", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(StringToIntNode::GetStaticType(), "String To Int", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(StringToBoolNode::GetStaticType(), "String To Bool", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(StringToVectorNode::GetStaticType(), "String To Vector", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(StringToColorNode::GetStaticType(), "String To Color", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));

    // Conversion nodes - Type
    AddNodeType(VectorToColorNode::GetStaticType(), "Vector To Color", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(ColorToVectorNode::GetStaticType(), "Color To Vector", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(SplitVectorNode::GetStaticType(), "Split Vector", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(SplitColorNode::GetStaticType(), "Split Color", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));

    // String nodes
    AddNodeType(StringConcatenateNode::GetStaticType(), "String Concatenate", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringLengthNode::GetStaticType(), "String Length", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(SubstringNode::GetStaticType(), "Substring", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringContainsNode::GetStaticType(), "String Contains", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringFindNode::GetStaticType(), "String Find", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringReplaceNode::GetStaticType(), "String Replace", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringSplitNode::GetStaticType(), "String Split", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringToUpperNode::GetStaticType(), "String To Upper", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringToLowerNode::GetStaticType(), "String To Lower", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringTrimNode::GetStaticType(), "String Trim", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringFormatNode::GetStaticType(), "String Format", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringStartsWithNode::GetStaticType(), "String Starts With", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringEndsWithNode::GetStaticType(), "String Ends With", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));

    // Utility nodes (new)
    AddNodeType(MakeVectorNode::GetStaticType(), "Make Vector", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(MakeVector2DNode::GetStaticType(), "Make Vector 2D", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Utility nodes
    AddNodeType(ViewerNode::GetStaticType(), "Viewer", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(DebugLogNode::GetStaticType(), "Debug Log", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Procedural nodes
    AddNodeType(ProceduralOutputNode::GetStaticType(), "Procedural Output", "Procedural", glm::vec4(0.8f, 0.5f, 0.1f, 1.0f));
    AddNodeType(NoiseNode::GetStaticType(), "Noise", "Procedural", glm::vec4(0.8f, 0.5f, 0.1f, 1.0f));
    AddNodeType(VoronoiNode::GetStaticType(), "Voronoi", "Procedural", glm::vec4(0.8f, 0.5f, 0.1f, 1.0f));
    AddNodeType(GradientNode::GetStaticType(), "Gradient", "Procedural", glm::vec4(0.8f, 0.5f, 0.1f, 1.0f));
}

TypeId ProceduralDomain::GetDefaultOutputNodeType() const
{
    return ProceduralOutputNode::GetStaticType();
}

void ProceduralDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // After evaluation, the ProceduralOutputNode's input pins contain the final procedural parameters.
}
