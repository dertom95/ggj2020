-- api: https://urho3d.github.io/documentation/1.7/_lua_script_a_p_i.html
-- events: https://urho3d.github.io/documentation/1.7/_event_list.html


function Start()
    fortuna="duesseldorf"

    SubscribeToEvent("Update","HandleUpdate")
end

function HandleUpdate(eventType,eventData)
     print("dt:"..eventData["TimeStep"])
end

Tester = ScriptObject()

function Tester:Start()
    self.userdata = self.node:GetVars()
    self.testValue = 10
    self:SubscribeToEvent("Update","Tester:HandleUpdate")
end

function Tester:HandleUpdate(eventType,eventData)
    local dt = eventData["TimeStep"]
    self.testValue = self.testValue + 1
    print("UserData from blender:"..self.userdata["prefix"]:GetString())
end






-- example collision detection
-- 
-- function HandleNodeCollision(eventType, eventData)
-- 
--     local nodeA = eventData["NodeA"]:GetPtr("Node")
--     local nodeB = eventData["NodeB"]:GetPtr("Node")
-- 
--     if (nodeA:GetName()=="Box") then
--        nodeB:Remove()     
--     elseif (nodeB:GetName()=="Box") then
--        nodeA:Remove()
--     end
--     --print ("LUA Collision:"..nodeA:GetName().."<->"..nodeB:GetName())
-- end
--    
-- SubscribeToEvent("PhysicsCollision", "HandleNodeCollision")

