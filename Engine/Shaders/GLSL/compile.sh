mkdir -p bin 
cd ./src

if command -v glslc >/dev/null 2>&1
then
    for file in *.vert *.frag *.comp
    do
        echo $VULKAN_SDK/bin/glslc $file -Os -o ../bin/$file
        val= $VULKAN_SDK/bin/glslc $file -Os -o ../bin/$file

        if [ val != 0 ]
        then
            echo "Compilation successful!"
        else
            read -n1 -r -p "Press any key to continue..."
        fi

    done
else
    echo "ERROR: glslc not detected - have you installed Shaderc? Try the LunarG Vulkan SDK!"
    exit 1;
fi

cd ..
