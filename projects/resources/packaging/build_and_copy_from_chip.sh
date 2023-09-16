ssh chip@pocketchip.local 'cd /home/chip/LittleGPTracker/projects/ \
&& git checkout o-release-candidate && git pull && make'
scp chip@pocketchip.local:/home/chip/LittleGPTracker/projects/lgpt.rpi-exe $(git rev-parse --show-toplevel)/projects/lgpt.chip-exe
