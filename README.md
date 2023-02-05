# Voxel Space Engine for Dreamcast
added my version of faster SQ and sh4 math header

Now 16fps on hardware 
![bandicam 2023-02-05 05-16-46-130](https://user-images.githubusercontent.com/59771322/216813317-a3fb3c31-c8cd-4324-8c95-ebef077d9653.jpg)

https://user-images.githubusercontent.com/59771322/212742671-7e54db6f-d949-476f-9f84-e73b5e3623d1.mp4

Dreamcast emulators are much faster then hardware examples below
King of being way too fast flycast
![flycast 2023-01-16 14-06-53-537](https://user-images.githubusercontent.com/59771322/212750682-476e49ea-c645-4721-9b84-62f439ac361f.jpg)
DEMUL 15fps 5 fps faster then a real dreamcast
![demul1 2023-01-16 13-55-28-050](https://user-images.githubusercontent.com/59771322/212748971-f5a3005c-5d03-4402-b330-2e579d5742bd.jpg)
Redream faster then hardware
![redream 2023-01-16 13-57-58-990](https://user-images.githubusercontent.com/59771322/212749154-8d3e3ff2-5ecb-42ce-926f-334d9e226ef4.jpg)
Nulldc 7 fps faster then hardware
![nullDC_Win32_Release 2023-01-16 13-58-54-075](https://user-images.githubusercontent.com/59771322/212749305-64959318-1136-4c52-951e-a3e8a72e1da0.jpg)
Update video






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
