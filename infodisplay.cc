// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "threaded-canvas-manipulator.h"

#include "convertstringtopixel.h"

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>

using std::min;
using std::max;

using namespace rgb_matrix;

// This is an example how to use the Canvas abstraction to map coordinates.
//
// This is a Canvas that delegates to some other Canvas (typically, the RGB
// matrix).
//
// Here, we want to address four 32x32 panels as one big 64x64 panel. Physically,
// we chain them together and do a 180 degree 'curve', somewhat like this:
// [>] [>]
//         v
// [<] [<]
class LargeSquare64x64Canvas : public Canvas {
public:
  // This class takes over ownership of the delegatee.
  LargeSquare64x64Canvas(Canvas *delegatee) : delegatee_(delegatee) {
    // Our assumptions of the underlying geometry:
    assert(delegatee->height() == 32);
    assert(delegatee->width() == 128);
  }
  virtual ~LargeSquare64x64Canvas() { delete delegatee_; }

  virtual void Clear() { delegatee_->Clear(); }
  virtual void Fill(uint8_t red, uint8_t green, uint8_t blue) {
    delegatee_->Fill(red, green, blue);
  }
  virtual int width() const { return 64; }
  virtual int height() const { return 64; }
  virtual void SetPixel(int x, int y,
                        uint8_t red, uint8_t green, uint8_t blue) {
    if (x < 0 || x >= width() || y < 0 || y >= height()) return;
    // We have up to column 64 one direction, then folding around. Lets map
    if (y > 31) {
      x = 127 - x;
      y = 63 - y;
    }
    delegatee_->SetPixel(x, y, red, green, blue);
  }

private:
  Canvas *delegatee_;
};



class ImageScroller : public ThreadedCanvasManipulator {
public:
  // Scroll image with "scroll_jumps" pixels every "scroll_ms" milliseconds.
  // If "scroll_ms" is negative, don't do any scrolling.
  ImageScroller(Canvas *m, int scroll_jumps, int scroll_ms = 30)
    : ThreadedCanvasManipulator(m), scroll_jumps_(scroll_jumps),
      scroll_ms_(scroll_ms),
      horizontal_position_(0) {
  }

  virtual ~ImageScroller() {
    Stop();
    WaitStopped();   // only now it is safe to delete our instance variables.
  }

  // _very_ simplified. Can only read binary P6 PPM. Expects newlines in headers
  // Not really robust. Use at your own risk :)
  // This allows reload of an image while things are running, e.g. you can
  // life-update the content.
  bool LoadPPM(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return false;
    char header_buf[256];
    const char *line = ReadLine(f, header_buf, sizeof(header_buf));
#define EXIT_WITH_MSG(m) { fprintf(stderr, "%s: %s |%s", filename, m, line); \
      fclose(f); return false; }
    if (sscanf(line, "P6 ") == EOF)
      EXIT_WITH_MSG("Can only handle P6 as PPM type.");
    line = ReadLine(f, header_buf, sizeof(header_buf));
    int new_width, new_height;
    if (!line || sscanf(line, "%d %d ", &new_width, &new_height) != 2)
      EXIT_WITH_MSG("Width/height expected");
    int value;
    line = ReadLine(f, header_buf, sizeof(header_buf));
    if (!line || sscanf(line, "%d ", &value) != 1 || value != 255)
      EXIT_WITH_MSG("Only 255 for maxval allowed.");
    const size_t pixel_count = new_width * new_height;
    Pixel *new_image = new Pixel [ pixel_count ];
    assert(sizeof(Pixel) == 3);   // we make that assumption.
    if (fread(new_image, sizeof(Pixel), pixel_count, f) != pixel_count) {
      line = "";
      EXIT_WITH_MSG("Not enough pixels read.");
    }
#undef EXIT_WITH_MSG
    fclose(f);
    fprintf(stderr, "Read image '%s' with %dx%d\n", filename,
            new_width, new_height);
    horizontal_position_ = 0;
    MutexLock l(&mutex_new_image_);
    new_image_.Delete();  // in case we reload faster than is picked up
    new_image_.image = new_image;
    new_image_.width = new_width;
    new_image_.height = new_height;
    return true;
  }
  
  bool ConvertStringToBits(char *t) {
    
    int new_width, new_height;
	int len = 0;
	
	len = strlen(t);
	char* pixzels = NULL;
	
	pixzels=convertStringToBits(t,pixzels);
	
	
    
    //const size_t pixel_count = new_width * new_height;
	new_height = MATRIXHEIGHT;
	new_width = len * MATRIXCHARWIDTH;
	const size_t pixel_count = len * MATRIXCHARWIDTH * MATRIXHEIGHT;
	printf("count : %d\n",pixel_count);
	
	
    Pixel *new_image = new Pixel [ pixel_count ];
    assert(sizeof(Pixel) == 3);   // we make that assumption.
    // if (fread(new_image, sizeof(Pixel), pixel_count, f) != pixel_count) {
      // line = "";
      // EXIT_WITH_MSG("Not enough pixels read.");
    // }
	
	int i = 0;
	for(i=0 ; i<pixel_count;i++) {
		if(pixzels[i]) {
				// printf("#%03d",i);
				printf("#");
				new_image[i].red = 0xFF;
				new_image[i].green = 0xFF;
				new_image[i].blue = 0xFF;
			}
			else {
				// printf(" %03d",i);
				printf(" ");
				new_image[i].red = 0xFF;
				new_image[i].green = 0xFF;
				new_image[i].blue = 0xFF;
			}
			if(i%(len*MATRIXCHARWIDTH) ==0)
			printf("\n");
	}
	printf("\n");
	
    horizontal_position_ = 0;
    MutexLock l(&mutex_new_image_);
    new_image_.Delete();  // in case we reload faster than is picked up
    new_image_.image = new_image;
    new_image_.width = new_width;
    new_image_.height = new_height;
    return true;
  }

  void Run() {
    const int screen_height = canvas()->height();
    const int screen_width = canvas()->width();
    while (running()) {
      {
        MutexLock l(&mutex_new_image_);
        if (new_image_.IsValid()) {
          current_image_.Delete();
          current_image_ = new_image_;
          new_image_.Reset();
        }
      }
      if (!current_image_.IsValid()) {
        usleep(100 * 1000);
        continue;
      }
      for (int x = 0; x < screen_width; ++x) {
        for (int y = 0; y < screen_height; ++y) {
          const Pixel &p = current_image_.getPixel(
                     (horizontal_position_ + x) % current_image_.width, y);
          canvas()->SetPixel(x, y, p.red, p.green, p.blue);
        }
      }
      horizontal_position_ += scroll_jumps_;
      if (horizontal_position_ < 0) horizontal_position_ = current_image_.width;
      if (scroll_ms_ <= 0) {
        // No scrolling. We don't need the image anymore.
        current_image_.Delete();
      } else {
        usleep(scroll_ms_ * 1000);
      }
    }
  }

private:
  struct Pixel {
    Pixel() : red(0), green(0), blue(0){}
    uint8_t red;
    uint8_t green;
    uint8_t blue;
  };

  struct Image {
    Image() : width(-1), height(-1), image(NULL) {}
    ~Image() { Delete(); }
    void Delete() { delete [] image; Reset(); }
    void Reset() { image = NULL; width = -1; height = -1; }
    inline bool IsValid() { return image && height > 0 && width > 0; }
    const Pixel &getPixel(int x, int y) {
      static Pixel black;
      if (x < 0 || x >= width || y < 0 || y >= height) return black;
      return image[x + width * y];
    }

    int width;
    int height;
    Pixel *image;
  };

  // Read line, skip comments.
  char *ReadLine(FILE *f, char *buffer, size_t len) {
    char *result;
    do {
      result = fgets(buffer, len, f);
    } while (result != NULL && result[0] == '#');
    return result;
  }

  const int scroll_jumps_;
  const int scroll_ms_;

  // Current image is only manipulated in our thread.
  Image current_image_;

  // New image can be loaded from another thread, then taken over in main thread.
  Mutex mutex_new_image_;
  Image new_image_;

  int32_t horizontal_position_;
};






// static int usage(const char *progname) {
  // fprintf(stderr, "usage: %s <options> -D <demo-nr> [optional parameter]\n",
          // progname);
  // fprintf(stderr, "Options:\n"
          // "\t-r <rows>     : Display rows. 16 for 16x32, 32 for 32x32. "
          // "Default: 32\n"
          // "\t-P <parallel> : For Plus-models or RPi2: parallel chains. 1..3. "
          // "Default: 1\n"
          // "\t-c <chained>  : Daisy-chained boards. Default: 1.\n"
          // "\t-L            : 'Large' display, composed out of 4 times 32x32\n"
          // "\t-p <pwm-bits> : Bits used for PWM. Something between 1..11\n"
          // "\t-j            : Low jitter. Experimental. Only RPi2\n"
          // "\t-l            : Don't do luminance correction (CIE1931)\n"
          // "\t-D <demo-nr>  : Always needs to be set\n"
          // "\t-d            : run as daemon. Use this when starting in\n"
          // "\t                /etc/init.d, but also when running without\n"
          // "\t                terminal (e.g. cron).\n"
          // "\t-t <seconds>  : Run for these number of seconds, then exit.\n"
          // "\t       (if neither -d nor -t are supplied, waits for <RETURN>)\n");
  // fprintf(stderr, "Demos, choosen with -D\n");
  // fprintf(stderr, "\t0  - some rotating square\n"
          // "\t1  - forward scrolling an image (-m <scroll-ms>)\n"
          // "\t2  - backward scrolling an image (-m <scroll-ms>)\n"
          // "\t3  - test image: a square\n"
          // "\t4  - Pulsing color\n"
          // "\t5  - Grayscale Block\n"
          // "\t6  - Abelian sandpile model (-m <time-step-ms>)\n"
          // "\t7  - Conway's game of life (-m <time-step-ms>)\n"
          // "\t8  - Langton's ant (-m <time-step-ms>)\n"
          // "\t9  - Volume bars (-m <time-step-ms>)\n");
  // fprintf(stderr, "Example:\n\t%s -t 10 -D 1 runtext.ppm\n"
          // "Scrolls the runtext for 10 seconds\n", progname);
  // return 1;
// }

int main(int argc, char *argv[]) {
  bool as_daemon = false;
  int runtime_seconds = -1;
  int demo = -1;
  int rows = 32;
  int chain = 1;
  int parallel = 1;
  int scroll_ms = 30;
  int pwm_bits = -1;
  bool large_display = false;
  bool do_luminance_correct = true;

  const char *demo_parameter = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "dlD:t:r:P:c:p:m:L")) != -1) {
    switch (opt) {
    case 'D':
      demo = atoi(optarg);
      break;

    case 'd':
      as_daemon = true;
      break;

    case 't':
      runtime_seconds = atoi(optarg);
      break;

    case 'r':
      rows = atoi(optarg);
      break;

    case 'P':
      parallel = atoi(optarg);
      break;

    case 'c':
      chain = atoi(optarg);
      break;

    case 'm':
      scroll_ms = atoi(optarg);
      break;

    case 'p':
      pwm_bits = atoi(optarg);
      break;

    case 'l':
      do_luminance_correct = !do_luminance_correct;
      break;

    case 'L':
      // The 'large' display assumes a chain of four displays with 32x32
      chain = 4;
      rows = 32;
      large_display = true;
      break;

    default: /* '?' */
	break;
    }
  }

  if (optind < argc) {
    demo_parameter = argv[optind];
  }

  if (demo < 0) {
    fprintf(stderr, "Expected required option -D <demo>\n");
    return 0;//usage(argv[0]);
  }

  if (getuid() != 0) {
    fprintf(stderr, "Must run as root to be able to access /dev/mem\n"
            "Prepend 'sudo' to the command:\n\tsudo %s ...\n", argv[0]);
    return 1;
  }

  if (rows != 16 && rows != 32) {
    fprintf(stderr, "Rows can either be 16 or 32\n");
    return 1;
  }

  if (chain < 1) {
    fprintf(stderr, "Chain outside usable range\n");
    return 1;
  }
  if (chain > 8) {
    fprintf(stderr, "That is a long chain. Expect some flicker.\n");
  }
  if (parallel < 1 || parallel > 3) {
    fprintf(stderr, "Parallel outside usable range.\n");
    return 1;
  }

  // Initialize GPIO pins. This might fail when we don't have permissions.
  GPIO io;
  if (!io.Init())
    return 1;

  // Start daemon before we start any threads.
  if (as_daemon) {
    if (fork() != 0)
      return 0;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }

  // The matrix, our 'frame buffer' and display updater.
  RGBMatrix *matrix = new RGBMatrix(&io, rows, chain, parallel);
  matrix->set_luminance_correct(do_luminance_correct);
  if (pwm_bits >= 0 && !matrix->SetPWMBits(pwm_bits)) {
    fprintf(stderr, "Invalid range of pwm-bits\n");
    return 1;
  }

  Canvas *canvas = matrix;

  if (large_display) {
    // Mapping the coordinates of a 32x128 display mapped to a square of 64x64
    canvas = new LargeSquare64x64Canvas(canvas);
  }

  // The ThreadedCanvasManipulator objects are filling
  // the matrix continuously.
  ThreadedCanvasManipulator *image_gen = NULL;
  switch (demo) {
  case 1:
  case 2:
    if (demo_parameter) {
      ImageScroller *scroller = new ImageScroller(canvas,
                                                  demo == 1 ? 1 : -1,
                                                  scroll_ms);
      //if (!scroller->LoadPPM(demo_parameter))
	  if (!scroller->ConvertStringToBits((char*)demo_parameter))
        return 1;
      image_gen = scroller;
    } else {
      fprintf(stderr, "Demo %d Requires PPM image as parameter\n", demo);
      return 1;
    }
    break;

  }

  // if (image_gen == NULL)
    // return usage(argv[0]);

  // Image generating demo is crated. Now start the thread.
  image_gen->Start();

  // Now, the image genreation runs in the background. We can do arbitrary
  // things here in parallel. In this demo, we're essentially just
  // waiting for one of the conditions to exit.
  if (as_daemon) {
    sleep(runtime_seconds > 0 ? runtime_seconds : INT_MAX);
  } else if (runtime_seconds > 0) {
    sleep(runtime_seconds);
  } else {
    // Things are set up. Just wait for <RETURN> to be pressed.
    printf("Press <RETURN> to exit and reset LEDs\n");
    getchar();
  }
  
  //convertStringToBits((char*)"toto");

  // Stop image generating thread.
  delete image_gen;
  delete canvas;

  return 0;
}
