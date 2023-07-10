ssh chip@pocketchip.local 'cd /home/chip/LittleGPTracker/projects/ \
&& git checkout o-release-candidate && git pull && make'
scp chip@pocketchip.local:/home/chip/LittleGPTracker/projects/lgpt.rpi-exe /home/m/Documents/GitRepos/LittleGPTracker/projects/lgpt.chip-exe