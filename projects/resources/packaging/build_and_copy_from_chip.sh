ssh chip@pocketchip.local 'cd /home/chip/Documents/LittleGPTracker/projects/ \
&& git checkout o-release-candidate && git pull && make'
scp chip@pocketchip.local:/home/chip/Documents/LittleGPTracker/projects/lgpt.rpi-exe $(git rev-parse --show-toplevel)/projects/lgpt.chip-exe
