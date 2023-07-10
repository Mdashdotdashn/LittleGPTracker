ssh pi@retropie.local 'cd /home/pi/RetroPie/roms/ports/LittleGPTracker-FeatureImprovements/projects/ \
&& git checkout o-release-candidate && git pull && make PLATFORM=RASPI'
scp pi@retropie.local:/home/pi/RetroPie/roms/ports/LittleGPTracker-FeatureImprovements/projects/lgpt.rpi-exe /home/m/Documents/GitRepos/LittleGPTracker/projects/