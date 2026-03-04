#include "NodeGraph/Domains/ProceduralDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/ProceduralNodes.h"
#include "NodeGraph/Nodes/PointNodes.h"
#include "NodeGraph/Nodes/SplineNodes.h"
#include "NodeGraph/Nodes/SceneGraphNodes.h"

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

    // Math nodes - Vector Arithmetic
    AddNodeType(AddVectorNode::GetStaticType(), "Add Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SubtractVectorNode::GetStaticType(), "Subtract Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyVectorNode::GetStaticType(), "Multiply Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(DivideVectorNode::GetStaticType(), "Divide Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyVectorComponentNode::GetStaticType(), "Multiply Vector Component", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(LerpVectorNode::GetStaticType(), "Lerp Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(NegateVectorNode::GetStaticType(), "Negate Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

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

    // Event nodes (for realtime procedural use)
    AddNodeType(StartEventNode::GetStaticType(), "Start", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    AddNodeType(TickEventNode::GetStaticType(), "Tick", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    AddNodeType(StopEventNode::GetStaticType(), "Stop", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));

    // Flow Control nodes
    AddNodeType(BranchNode::GetStaticType(), "Branch", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(ForLoopNode::GetStaticType(), "For Loop", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(ForEachLoopNode::GetStaticType(), "For Each Loop", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(SequenceNode::GetStaticType(), "Sequence", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    // Point Generation nodes
    static const glm::vec4 kPointColor(0.9f, 0.55f, 0.1f, 1.0f);
    AddNodeType(CreatePointsGridNode::GetStaticType(), "Create Points Grid", "Point", kPointColor);
    AddNodeType(ScatterPointsOnMeshNode::GetStaticType(), "Scatter Points On Mesh", "Point", kPointColor);
    AddNodeType(PointsFromSplineNode::GetStaticType(), "Points From Spline", "Point", kPointColor);
    AddNodeType(CreatePointsLineNode::GetStaticType(), "Create Points Line", "Point", kPointColor);
    AddNodeType(RandomPointsInBoxNode::GetStaticType(), "Random Points In Box", "Point", kPointColor);
    AddNodeType(MergePointsNode::GetStaticType(), "Merge Points", "Point", kPointColor);

    // Point Attribute nodes
    AddNodeType(SetPointAttributeFloatNode::GetStaticType(), "Set Point Attribute (Float)", "Point", kPointColor);
    AddNodeType(SetPointAttributeVectorNode::GetStaticType(), "Set Point Attribute (Vector)", "Point", kPointColor);
    AddNodeType(SetPointAttributeColorNode::GetStaticType(), "Set Point Attribute (Color)", "Point", kPointColor);
    AddNodeType(GetPointAttributeFloatNode::GetStaticType(), "Get Point Attribute (Float)", "Point", kPointColor);
    AddNodeType(GetPointAttributeVectorNode::GetStaticType(), "Get Point Attribute (Vector)", "Point", kPointColor);
    AddNodeType(GetPointAttributeColorNode::GetStaticType(), "Get Point Attribute (Color)", "Point", kPointColor);
    AddNodeType(GetPointCountNode::GetStaticType(), "Get Point Count", "Point", kPointColor);
    AddNodeType(GetPointPositionNode::GetStaticType(), "Get Point Position", "Point", kPointColor);
    AddNodeType(SetPointPositionNode::GetStaticType(), "Set Point Position", "Point", kPointColor);

    // Point Iteration / Transform nodes
    AddNodeType(ForEachPointNode::GetStaticType(), "For Each Point", "Point", kPointColor);
    AddNodeType(SetPointInLoopNode::GetStaticType(), "Set Point In Loop", "Point", kPointColor);
    AddNodeType(FilterPointsNode::GetStaticType(), "Filter Points", "Point", kPointColor);
    AddNodeType(TransformPointsNode::GetStaticType(), "Transform Points", "Point", kPointColor);
    AddNodeType(RandomizeAttributeNode::GetStaticType(), "Randomize Attribute", "Point", kPointColor);

    // Point Output / Bridge nodes
    AddNodeType(CopyToPointsNode::GetStaticType(), "Copy To Points", "Point", kPointColor);
    AddNodeType(InstantiateAtPointsNode::GetStaticType(), "Instantiate At Points", "Point", kPointColor);
    AddNodeType(PointCloudToProceduralOutputNode::GetStaticType(), "Points To Output", "Point", kPointColor);

    // Point Utility nodes
    AddNodeType(PointCloudFirstPointNode::GetStaticType(), "First Point", "Point", kPointColor);
    AddNodeType(PointCloudLastPointNode::GetStaticType(), "Last Point", "Point", kPointColor);
    AddNodeType(RemovePointNode::GetStaticType(), "Remove Point", "Point", kPointColor);
    AddNodeType(ReversePointsNode::GetStaticType(), "Reverse Points", "Point", kPointColor);
    AddNodeType(SortPointsByAttributeNode::GetStaticType(), "Sort Points", "Point", kPointColor);

    // Spline nodes
    static const glm::vec4 kSplineColor(0.3f, 0.7f, 0.9f, 1.0f);
    AddNodeType(SplinePositionAtNode::GetStaticType(), "Spline Position At", "Spline", kSplineColor);
    AddNodeType(SplineTangentAtNode::GetStaticType(), "Spline Tangent At", "Spline", kSplineColor);
    AddNodeType(SplinePointCountNode::GetStaticType(), "Spline Point Count", "Spline", kSplineColor);
    AddNodeType(SplineGetPointNode::GetStaticType(), "Spline Get Point", "Spline", kSplineColor);
    AddNodeType(SplineNearestPercentNode::GetStaticType(), "Spline Nearest Percent", "Spline", kSplineColor);
    AddNodeType(SplineLengthNode::GetStaticType(), "Spline Length", "Spline", kSplineColor);


    AddNodeType(GetNode3DName::GetStaticType(), "Get Node3D Name", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetNode3DClass::GetStaticType(), "Get Node3D Class", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
}

TypeId ProceduralDomain::GetDefaultOutputNodeType() const
{
    return ProceduralOutputNode::GetStaticType();
}

void ProceduralDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // After evaluation, the ProceduralOutputNode's input pins contain the final procedural parameters.
}
