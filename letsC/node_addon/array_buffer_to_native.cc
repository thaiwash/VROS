#include <napi.h>
#include <stdio.h>
#include <linux/fb.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
int x = 0, y = 0;
long int location = 0;

void init_framebuffer() {
    // Open the file for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        perror("Error reading fixed information");
        exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        perror("Error reading variable information");
        exit(3);
    }

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        exit(4);
    }
    //printf("The framebuffer device was mapped to memory successfully.\n");
}

void set_pixels(const int32_t* array) {
    int count = 0;
    
    for (y = 0; y < 2560; y++) {
        for (x = 0; x < 1440; x++) {

            location = (x) * (vinfo.bits_per_pixel/8) +
                       (y) * finfo.line_length;
                       

                int b = array[count+2];
                int g = array[count+1];     
                int r = array[count]; 
                
                count += 3;
                
                unsigned short int t = r<<11 | g << 5 | b;
                *((unsigned short int*)(fbp + location)) = t;

        }
    }
}

void close_framebuffer() {
    munmap(fbp, screensize);
    close(fbfd);
}

static void ArrayConsumer(const int32_t* array, const size_t length) {
  if (length != 1440*2560*3) {
    printf("Warning: Array size must fill the screen (must be exactly %i)", 1440*2560*3);
  } else {
    set_pixels(array);
  }
}

static Napi::Value AcceptArrayBuffer(const Napi::CallbackInfo& info) {
  if (info.Length() != 1) {
    Napi::Error::New(info.Env(), "Expected exactly one argument")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  if (!info[0].IsArrayBuffer()) {
    Napi::Error::New(info.Env(), "Expected an ArrayBuffer")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::ArrayBuffer buf = info[0].As<Napi::ArrayBuffer>();

  ArrayConsumer(reinterpret_cast<int32_t*>(buf.Data()),
                buf.ByteLength() / sizeof(int32_t));

  return info.Env().Undefined();
}

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
  init_framebuffer();
  exports["AcceptArrayBuffer"] = Napi::Function::New(env, AcceptArrayBuffer);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
