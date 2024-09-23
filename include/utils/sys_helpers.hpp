#include <sys/stat.h>
#include <sys/time.h>
#include <string>
#include <stdlib.h>
#include <cstdio>

namespace neuro {

/* get absolute path */
inline static std::string sys_abs_path(const std::string &rel_path) {
  char *abs_path;
  std::string rv;
  std::ofstream o;
  o.open(rel_path, std::ios::app);
  o.close();
  abs_path = realpath(rel_path.c_str(), nullptr);
  if (abs_path == nullptr) throw std::runtime_error("relpath failed - " + rel_path);
  rv = std::string(abs_path);
  free(abs_path);
  return rv;
}

/* Make the directory if it doesn't exist */
inline static void sys_make_dir(const std::string &dir) {
  struct stat buf;
  string s;
  if (stat(dir.c_str(), &buf) == -1) {
    if (mkdir(dir.c_str(), S_IRWXU) == -1) {
      s = (string) "Error: Failed to make the " + dir + "directory\n";
      throw std::runtime_error(s);
    }
  }
}


/* get the current time */
inline static double sys_get_time() {
  struct timeval tv;
  double rv;

  gettimeofday(&tv, NULL);
  rv = tv.tv_usec;
  rv /= 1000000.0;
  rv += tv.tv_sec;
  return rv;
}


inline static std::string sys_time_to_date(const int secs) {
  int h, m, s;
  char buf[128];
  h = secs / 3600;
  m = (secs - h * 3600) / 60;
  s = secs - h * 3600 - m * 60;
  snprintf(buf, 128, "%02d:%02d:%02d", h, m, s);
  return (std::string) buf;
}


}
