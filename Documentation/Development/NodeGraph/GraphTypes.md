# Graph Types

The node graph system supports 6 graph domain types. Each domain defines its own set of available nodes, a default output node, and a domain color.

## Material

**Domain Class:** `MaterialDomain`
**Color:** Red (0.7, 0.2, 0.2)
**Output Node:** `MaterialOutputNode`

Purpose: Define material properties for rendering. Connect texture samples, math operations, and value nodes to produce material parameters.

**Domain-Specific Nodes:**
- **Material Output** - Sink node. Inputs: Base Color, Metallic, Roughness, Normal, Emissive, Opacity.
- **Texture Sample** - Sample a texture at UV coordinates. Outputs: Color, R, G, B, A.
- **Fresnel** - Fresnel effect approximation. Inputs: Exponent, Base Value. Output: Result.
- **Panner** - Animate UV coordinates. Inputs: UV, Speed X/Y, Time. Output: UV.
- **Normal Map** - Unpack and scale normal map data. Inputs: Normal, Strength. Output: Result.

**Shared Nodes:** All Math nodes, all Value nodes, Viewer.

---

## Shader

**Domain Class:** `ShaderDomain`
**Color:** Blue (0.2, 0.4, 0.8)
**Output Node:** `ShaderOutputNode`

Purpose: Define custom shader logic with vertex and fragment stage parameters.

**Domain-Specific Nodes:**
- **Shader Output** - Sink node. Inputs: Color, Alpha, Vertex Offset, World Normal.
- **Vertex Position** - Provides the vertex position. Output: Position.
- **Vertex Normal** - Provides the vertex normal. Output: Normal.
- **World Position** - Provides the world-space position. Output: Position.

**Shared Nodes:** All Math nodes, all Value nodes, Viewer.

---

## Procedural

**Domain Class:** `ProceduralDomain`
**Color:** Orange (0.8, 0.5, 0.1)
**Output Node:** `ProceduralOutputNode`

Purpose: Procedural content generation for terrain, textures, and effects.

**Domain-Specific Nodes:**
- **Procedural Output** - Sink node. Inputs: Height, Density, Color, Mask.
- **Noise** - Generate noise values. Inputs: Position, Scale, Octaves. Output: Value.
- **Voronoi** - Generate Voronoi patterns. Inputs: Position, Scale. Outputs: Distance, Cell ID.
- **Gradient** - Interpolate between two colors. Inputs: Position, Start, End. Output: Color.

**Shared Nodes:** All Math nodes, all Value nodes, Viewer.

---

## Animation

**Domain Class:** `AnimationDomain`
**Color:** Green (0.2, 0.7, 0.3)
**Output Node:** `AnimationOutputNode`

Purpose: Define animation blend trees and playback control.

**Domain-Specific Nodes:**
- **Animation Output** - Sink node. Inputs: Pose, Blend Weight, Speed.
- **Anim Clip** - Represents an animation clip. Outputs: Pose, Duration.
- **Blend** - Blend between two poses. Inputs: Pose A, Pose B, Alpha. Output: Result.
- **Anim Speed** - Scale animation playback time. Inputs: Speed, Time. Output: Scaled Time.

**Shared Nodes:** Add, Subtract, Multiply, Divide, Lerp, Clamp, Float, Integer, Time, Viewer.

---

## FSM (Finite State Machine)

**Domain Class:** `FSMDomain`
**Color:** Purple (0.6, 0.2, 0.7)
**Output Node:** `FSMOutputNode`

Purpose: Define state machines with states, transitions, and conditions.

**Domain-Specific Nodes:**
- **FSM Output** - Sink node. Inputs: State ID, Transition.
- **State** - Represents a state. Input: Enter. Outputs: Active, State ID.
- **Transition** - Defines a state transition. Inputs: Condition, From State, To State. Output: Should Transition.
- **Condition** - Compares two values. Inputs: Value A, Value B. Output: Result (A > B).

**Shared Nodes:** Float, Integer, Viewer.

---

## SceneGraph

**Domain Class:** `SceneGraphDomain`
**Color:** Teal (0.1, 0.6, 0.6)
**Output Node:** `SceneGraphOutputNode`

Purpose: Manipulate scene node transforms and hierarchy.

**Domain-Specific Nodes:**
- **Scene Graph Output** - Sink node. Inputs: Transform, Active.
- **Get Transform** - Get a node's transform. Outputs: Position, Rotation, Scale.
- **Set Transform** - Set a node's transform. Inputs: Position, Rotation, Scale. Output: Transform.
- **Find Node** - Search for a node in the scene. Output: Found.

**Shared Nodes:** Add, Subtract, Multiply, Float, Integer, Vector, Viewer.
