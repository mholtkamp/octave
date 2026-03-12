-- DataAsset Definition Template
-- Copy this file to your project's Scripts folder and customize the properties
-- Then assign this script file to your DataAsset in the editor

-- Define properties that will appear in the editor inspector
function GetProperties()
    return {
        { name = "exampleFloat", type = DatumType.Float, default = 1.0 },
        { name = "exampleInt", type = DatumType.Integer, default = 0 },
        { name = "exampleString", type = DatumType.String, default = "" },
        { name = "exampleBool", type = DatumType.Bool, default = false },
        { name = "exampleVector", type = DatumType.Vector, default = {0, 0, 0} },
        { name = "exampleColor", type = DatumType.Color, default = {1, 1, 1, 1} },
        { name = "exampleAsset", type = DatumType.Asset },
        -- { name = "exampleArray", type = DatumType.Integer, array = true },
    }
end
