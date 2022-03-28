@echo off
bannertool.exe makebanner -i banner.png -a audio.wav -o banner.bnr
bannertool.exe makesmdh -s "Retro League GX" -l "Retro League GX" -p "Martin Holtkamp" -i icon.png  -o icon.icn
makerom -f cia -o homebrew.cia -DAPP_ENCRYPTED=false -rsf homebrew.rsf -target t -exefslogo -elf homebrew.elf -icon icon.icn -banner banner.bnr
makerom -f cci -o homebrew.3ds -DAPP_ENCRYPTED=true -rsf homebrew.rsf -target t -exefslogo -elf homebrew.elf -icon icon.icn -banner banner.bnr
echo Finished! 3DS and CIA have been built!
pause