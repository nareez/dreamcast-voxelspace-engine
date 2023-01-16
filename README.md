# Voxel Space Engine for Dreamcast

Update video
added my version of faster SQ and sh4 math header

https://user-images.githubusercontent.com/59771322/212742671-7e54db6f-d949-476f-9f84-e73b5e3623d1.mp4





## TODO

- [x] Develop workking code of voxel space engine for Dreamcast
- [x] FPS Count
- [ ] Collision




- [ ] Fog Effect
- [ ] Delta time

Issues:
- [x] Screen tearing and flickering - Solution: Develop Double Buffering
- [x] Slow RAM to VRAM data transfer - Solution: Transfer framebuffer data thru store queue copy
- [x] Ugly surface colors - Solution: Increase brightness
- [ ] Non aligned memory not allowing to run on real hardware

## Credits
* Pikuma's implementation https://github.com/gustavopezzi/voxelspace
* s-macke's implementation https://github.com/s-macke/VoxelSpace

## Dreamcast development credits
* dcemulation.org forum and wiki
* KallistiOS https://github.com/KallistiOS/KallistiOS
* Krejlooc tutorial on https://www.neogaf.com/threads/lets-build-a-sega-dreamcast-game-from-scratch-breakout.916501/
* Nold360's docker Dreamcast SDK for GCC 5 https://github.com/Nold360/docker-kallistios-sdk
* einsteinx2 docker Dreamcast SDK for GCC 9 https://github.com/einsteinx2/docker-dcdev
* Simulant Engine discord https://discord.com/invite/TRx94EV
* Dreamcast-talk https://www.dreamcast-talk.com/
