#ifndef _BASE64_H_
#define _BASE64_H_

int base64_encode(char *result, char *str, int size);
int base64_decode(char *result, char *str);

int base64_encode_padding(char *result, char *str, int size);
int base64_decode_padding(char *result, char *str);



#endif
