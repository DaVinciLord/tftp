#ifndef OPTIONS_H_
#define OPTIONS_H_

#define DEFAULT_WINDOWSIZE 1
#define DEFAULT_BLKSIZE 512

typedef struct {
    size_t blksize;
    size_t windowsize;
} options;


#endif // OPTIONS_H_ 
