-- 11_StateMachineDriver.lua
-- Advanced: Use a NodeGraph as a state machine driver.
-- Lua controls the high-level state transitions; the graph computes
-- per-state behavior (animation blending, material parameters, etc.)
--
-- Setup:
--   1. Create a NodeGraphAsset with:
--      - InputNodes: "StateIndex" (Int), "StateTime" (Float), "DeltaTime" (Float)
--      - Logic that branches on StateIndex to produce different outputs
--      - OutputNode: pin 0 (Float "BlendWeight"), pin 1 (Vector "Color")
--   2. Attach this script to a Node

StateMachineDriver = {}

-- State enum
local STATE_IDLE = 0
local STATE_ACTIVE = 1
local STATE_COOLDOWN = 2

function StateMachineDriver:Create()
    self.player = nil

    self.currentState = STATE_IDLE
    self.stateTime = 0.0
    self.activeDuration = 3.0
    self.cooldownDuration = 2.0
end

function StateMachineDriver:GatherProperties()
    return
    {
        { name = "graphAsset", type = DatumType.Asset },
        { name = "activeDuration", type = DatumType.Float },
        { name = "cooldownDuration", type = DatumType.Float }
    }
end

function StateMachineDriver:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)

    if self.graphAsset then
        self.player:SetNodeGraphAsset(self.graphAsset)
        self.player:Play()
    end

    self.currentState = STATE_IDLE
    self.stateTime = 0.0
end

function StateMachineDriver:Tick(deltaTime)
    if not self.player or not self.player:IsPlaying() then
        return
    end

    self.stateTime = self.stateTime + deltaTime

    -- State transitions (Lua logic)
    if self.currentState == STATE_IDLE then
        -- Transition to active after 5 seconds idle
        if self.stateTime >= 5.0 then
            self:ChangeState(STATE_ACTIVE)
        end

    elseif self.currentState == STATE_ACTIVE then
        if self.stateTime >= self.activeDuration then
            self:ChangeState(STATE_COOLDOWN)
        end

    elseif self.currentState == STATE_COOLDOWN then
        if self.stateTime >= self.cooldownDuration then
            self:ChangeState(STATE_IDLE)
        end
    end

    -- Feed state info into the graph
    self.player:SetInputInt("StateIndex", self.currentState)
    self.player:SetInputFloat("StateTime", self.stateTime)
    self.player:SetInputFloat("DeltaTime", deltaTime)

    -- Read computed results
    local blendWeight = self.player:GetOutputFloat(0)
    local color = self.player:GetOutputVector(1)

    -- Apply results
    local s = 1.0 + blendWeight * 0.5
    self:SetScale(Vec(s, s, s))
end

function StateMachineDriver:ChangeState(newState)
    local names = { [STATE_IDLE] = "IDLE", [STATE_ACTIVE] = "ACTIVE", [STATE_COOLDOWN] = "COOLDOWN" }
    Log.Debug(string.format("State: %s -> %s", names[self.currentState], names[newState]))
    self.currentState = newState
    self.stateTime = 0.0
end
