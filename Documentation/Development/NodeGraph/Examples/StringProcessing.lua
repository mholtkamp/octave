-- 14_StringProcessing.lua
-- Intermediate: Demonstrates string manipulation nodes.
--
-- Setup:
--   1. Create a SceneGraph NodeGraphAsset with:
--      - Two String InputNodes: "FirstName", "LastName"
--      - StringConcatenateNode: A=FirstName, B=" " + LastName (chain two concats)
--      - StringToUpperNode: Value=ConcatResult
--      - StringLengthNode: Value=ConcatResult
--      - StringContainsNode: String=ConcatResult, Search="OCTAVE"
--      - GraphOutput with pins: "FullName" (String), "UpperName" (String),
--        "NameLength" (Integer), "ContainsOctave" (Bool)
--   2. Attach this script, assign the asset, press Play

StringProcessing = {}

function StringProcessing:Create()
    self.player = nil
    self.nameIndex = 0
end

function StringProcessing:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset }
    }
end

function StringProcessing:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
        Log.Debug("String Processing demo started")
    end

    -- Sample names to cycle through
    self.names = {
        { first = "Octave",  last = "Engine" },
        { first = "Node",    last = "Graph" },
        { first = "Hello",   last = "World" },
    }
end

function StringProcessing:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    -- Cycle through names every 2 seconds
    self.nameIndex = (math.floor(self.player:GetOutputFloat(0) or 0) % #self.names) + 1
    -- Use a simpler counter approach
    self.nameIndex = (self.nameIndex % #self.names) + 1

    local entry = self.names[self.nameIndex]
    self.player:SetInputString("FirstName", entry.first)
    self.player:SetInputString("LastName", entry.last)

    -- Read string processing results (indices depend on your GraphOutput pin order)
    local fullName       = self.player:GetOutputString(0)
    local upperName      = self.player:GetOutputString(1)
    local nameLength     = self.player:GetOutputInt(2)
    local containsOctave = self.player:GetOutputBool(3)

    Log.Debug(string.format(
        "Full='%s' Upper='%s' Len=%d ContainsOctave=%s",
        fullName or "",
        upperName or "",
        nameLength or 0,
        tostring(containsOctave)
    ))
end
