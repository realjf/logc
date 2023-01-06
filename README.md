# logc

A simple header-only logging library in c

### Usage

```c

#define LOGC_IMPLEMENTATION
#define LOGC_USE_COLOR
#include "../include/logc.h"

int main() {
    log_trace("Hello %s", "world");

    char *ver = LOGC_VERSION;
    printf("%s", ver);
    free(ver);
    return 0;
}
```
