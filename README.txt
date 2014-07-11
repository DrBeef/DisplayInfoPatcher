
Simple really:

1. Copy the DisplayInfoPatcher.exe, RiftUp.DisplayInfo.bin and DK1.DisplayInfo.bin files to a folder
2. Open a command window, and cd to the folder
3. If you wish to back up your current DisplayInfo, you can use the following command (you could then compare to the one I provided)

DisplayInfoPatcher.exe -backup myRiftDK1Backup.bin

4. WHen you are ready, to write the RiftUp DIsplayInfo, use the following:

DisplayInfoPatcher.exe -write RiftUp.DisplayInfo.bin

5. There are some prompts, you accept full responsibility by saying yes, it will then update
6. You can then open the OculusRiftUtil and see that you are suddenly in the proud possesion of a pseudo DKHD2

7. In the event that you wish to restore the DK1 configuration, simply use the following:

DisplayInfoPatcher.exe -write DK1.DisplayInfo.bin

