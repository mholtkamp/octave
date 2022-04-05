mkdir -p bin 
cd ./src

if [[ "${VULKAN_SDK}" == "" ]] 
then
    echo "ERROR - Vulkan SDK not detected"
    exit 1;

else
    for file in *.vert *.frag
    do

        echo $VULKAN_SDK/bin/glslc $file -o ../bin/$file
        val= $VULKAN_SDK/bin/glslc $file -o ../bin/$file

        if [ val != 0 ]
        then
            echo Compile Successful
        else
            read -n1 -r -p "Press any key to continue..."
        fi

    done
fi

cd ..
