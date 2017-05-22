#include <stdio.h>
#include <string.h>

static unsigned char dtable[256];	      /* Encode / decode table */
static int errcheck = 0;	      /* Check decode input for errors ? */

/*  ENCODE  --	Encode into base64.  */
int base64_encode(char *result, char *str, int size)
{
    int i, hiteof = 0;
    int j = 0; /* to count the number of input, j <= size */
    int k = 0; /* to count the number of output */

    /*	Fill dtable with character encodings.  */

    for (i = 0; i < 26; i++) {
        dtable[i] = 'A' + i;
        dtable[26 + i] = 'a' + i;
    }
    for (i = 0; i < 10; i++) {
        dtable[52 + i] = '0' + i;
    }
    dtable[62] = '+';
    dtable[63] = '/';

    while (!hiteof) {
	unsigned char igroup[3], ogroup[4];
	int c, n;

	igroup[0] = igroup[1] = igroup[2] = 0;
	for (n = 0; n < 3; n++) { 
	    //c = *(str++);
	    c = str[j++];
	    //if (j == size){
	    //hiteof = 1;
	    //break;
	    //}
	    igroup[n] = (unsigned char) c;
	    if (j == size){
		hiteof = 1;
		break;
	    }		
	}

       ogroup[0] = dtable[igroup[0] >> 2];
       ogroup[1] = dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
       ogroup[2] = dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
       ogroup[3] = dtable[igroup[2] & 0x3F];

            /* Replace characters in output stream with "=" pad
	       characters if fewer than three characters were
	       read from the end of the input stream. */
#if 0 /* with padding */
	    if (n < 3) {
                ogroup[3] = '=';
		if (n < 2) {
                    ogroup[2] = '=';
		}
	    }
#endif
		if(hiteof){ /* no padding */
			switch (n){
			case 0:
				for(i = 0; i < 2; i++)
					result[k++] = ogroup[i];
				break;
			case 1:
				for(i = 0; i < 3; i++)
					result[k++] = ogroup[i];
				break;
			case 2:
			default:
				for(i = 0; i < 4; i++)
					result[k++] = ogroup[i];
			}
		}
		else{
			for (i = 0; i < 4; i++) {
				//*(result++) = ogroup[i];
				result[k++] = ogroup[i];
			}
		}
	}
    return k;
}

#if 1
int base64_decode(char *result, char *str)
{
    int i, j, c;
    int k = 0; /* to count the number of output */
    int l = 0;
    for (i = 0; i < 255; i++) {
        dtable[i] = 0x80;
    }
    for (i = 'A'; i <= 'Z'; i++) {
        dtable[i] = 0 + (i - 'A');
    }
    for (i = 'a'; i <= 'z'; i++) {
        dtable[i] = 26 + (i - 'a');
    }
    for (i = '0'; i <= '9'; i++) {
        dtable[i] = 52 + (i - '0');
    }
    dtable['+'] = 62;
    dtable['/'] = 63;
    dtable['='] = 0;

    /*CONSTANTCONDITION*/
    while (1) {
        unsigned char a[4], b[4], o[3];

        for (i = 0; i < 4; i++) {
            c = *(str++);
            
            if (c == '\0') {
                if (errcheck && (i > 0)) {
                        return;
                }
                //return; /* let it go for remaining encoded character */
                break; //stop when string end, khtee added 2016/07/17
            }
            if (dtable[c] & 0x80) {
                if (errcheck) {
                        return;
                }
                /* Ignoring errors: discard invalid character. */
                i--;
                continue;
            }
                l++;
            a[i] = (unsigned char) c;
            b[i] = (unsigned char) dtable[c];
        }

        o[0] = (b[0] << 2) | (b[1] >> 4);
        o[1] = (b[1] << 4) | (b[2] >> 2);
        o[2] = (b[2] << 6) | b[3];
        i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);

        for(j = 0; j < i; j++)
                //*(result++) = o[j];
                {
                        result[k++] = o[j];
                }
        if ((c == '\0')) { //change from i<3 to null char check , khtee added 2016/07/17
                            //return;
            break; /* for return value */
        }
    }

    return ((3*l))/4; //add l to record original string length and the content in string should be 3/4 length only, khtee added 2016/07/17

}
#else
/*  DECODE  --	Decode base64.	*/
int base64_decode(char *result, char *str)
{
    int i, j, c;
    int k = 0; /* to count the number of output */
    int l=0;
    for (i = 0; i < 255; i++) {
	dtable[i] = 0x80;
    }
    for (i = 'A'; i <= 'Z'; i++) {
        dtable[i] = 0 + (i - 'A');
    }
    for (i = 'a'; i <= 'z'; i++) {
        dtable[i] = 26 + (i - 'a');
    }
    for (i = '0'; i <= '9'; i++) {
        dtable[i] = 52 + (i - '0');
    }
    dtable['+'] = 62;
    dtable['/'] = 63;
    dtable['='] = 0;

    /*CONSTANTCONDITION*/
    while (1) {
	unsigned char a[4], b[4], o[3];

	for (i = 0; i < 4; i++) {
	    c = *(str++);
	    if (c == '\0') {
		if (errcheck && (i > 0)) {
			return;
		}
		//return; /* let it go for remaining encoded character */
		break; //stop when string end, khtee added 2016/07/17
	    }
	    if (dtable[c] & 0x80) {
		if (errcheck) {
			return;
		}
		/* Ignoring errors: discard invalid character. */
		i--;
		continue;
	    }
		l++;
	    a[i] = (unsigned char) c;
	    b[i] = (unsigned char) dtable[c];
	}

	o[0] = (b[0] << 2) | (b[1] >> 4);
	o[1] = (b[1] << 4) | (b[2] >> 2);
	o[2] = (b[2] << 6) | b[3];
        i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);

	for(j = 0; j < i; j++)
		//*(result++) = o[j];
		{
			result[k++] = o[j];
		}
	if ((c == '\0')) { //change from i<3 to null char check , khtee added 2016/07/17
			    //return;
	    break; /* for return value */
	}
    }

    return ((3*l))/4; //add l to record original string length and the content in string should be 3/4 length only, khtee added 2016/07/17

}
#endif


int base64_encode_padding(char *result, char *str, int size)
{
    result[0] = 0;
    if(size == 0) return 0;
    
    int i, hiteof = 0;
    int j = 0; /* to count the number of input, j <= size */
    int k = 0; /* to count the number of output */

    /*	Fill dtable with character encodings.  */

    for (i = 0; i < 26; i++) {
        dtable[i] = 'A' + i;
        dtable[26 + i] = 'a' + i;
    }
    for (i = 0; i < 10; i++) {
        dtable[52 + i] = '0' + i;
    }
    dtable[62] = '+';
    dtable[63] = '/';

    while (!hiteof) {
	unsigned char igroup[3], ogroup[4];
	int c, n;

	igroup[0] = igroup[1] = igroup[2] = 0;
	for (n = 0; n < 3; n++) { 
	    //c = *(str++);
	    c = str[j++];
	    //if (j == size){
	    //hiteof = 1;
	    //break;
	    //}
	    igroup[n] = (unsigned char) c;
	    if (j == size){
		hiteof = 1;
		n++; // for padding judge
		break;
	    }		
	}

       ogroup[0] = dtable[igroup[0] >> 2];
       ogroup[1] = dtable[((igroup[0] & 3) << 4) | (igroup[1] >> 4)];
       ogroup[2] = dtable[((igroup[1] & 0xF) << 2) | (igroup[2] >> 6)];
       ogroup[3] = dtable[igroup[2] & 0x3F];

            /* Replace characters in output stream with "=" pad
	       characters if fewer than three characters were
	       read from the end of the input stream. */
		if(hiteof){ /* no padding */
			#if 1/* with padding */
	 		   if (n < 3) {
             			   ogroup[3] = '=';
				if (n < 2) {
                    		ogroup[2] = '=';
			}
	    	}
		#endif

		}
		
		for (i = 0; i < 4; i++) {
			//*(result++) = ogroup[i];
			result[k++] = ogroup[i];
		}
		
	}
    result[k] = 0;
    return k;
}

/*  DECODE  --	Decode base64.	*/
int base64_decode_padding(char *result, char *str)
{
    int i, j, c;
    int k = 0; /* to count the number of output */
    int l = 0;
    int e=0;
    for (i = 0; i < 255; i++) {
        dtable[i] = 0x80;
    }
    for (i = 'A'; i <= 'Z'; i++) {
        dtable[i] = 0 + (i - 'A');
    }
    for (i = 'a'; i <= 'z'; i++) {
        dtable[i] = 26 + (i - 'a');
    }
    for (i = '0'; i <= '9'; i++) {
        dtable[i] = 52 + (i - '0');
    }
    dtable['+'] = 62;
    dtable['/'] = 63;
    dtable['='] = 0;

    /*CONSTANTCONDITION*/
    while (1) {
        unsigned char a[4], b[4], o[3];

        for (i = 0; i < 4; i++) {
            c = *(str++);
            
            if (c == '\0') {
                if (errcheck && (i > 0)) {
                        return;
                }
		
                //return; /* let it go for remaining encoded character */
                break; //stop when string end, khtee added 2016/07/17
            }

		if(c=='=')
			e++;
		
            if (dtable[c] & 0x80) {
                if (errcheck) {
                        return;
                }
                /* Ignoring errors: discard invalid character. */
                i--;
                continue;
            }
                l++;
            a[i] = (unsigned char) c;
            b[i] = (unsigned char) dtable[c];
        }

        o[0] = (b[0] << 2) | (b[1] >> 4);
        o[1] = (b[1] << 4) | (b[2] >> 2);
        o[2] = (b[2] << 6) | b[3];
        i = a[2] == '=' ? 1 : (a[3] == '=' ? 2 : 3);
	
        for(j = 0; j < i; j++)
                //*(result++) = o[j];
                {
                        result[k++] = o[j];
                }
        if ((c == '\0')) { //change from i<3 to null char check , khtee added 2016/07/17
                            //return;
            break; /* for return value */
        }
    }
  //printf("k:%d, l:%d,j:%d,i:%d,e:%d,r:%d\n",k,l,j,i,e,(((l*3)/4)-e));
    return (((l*3)/4)-e); //result[k++] , k already plus 1 

}

#if 0
/*  Main program  */
int main(int argc, char *argv[])
{
        char str[1024], encoded_str[1024], decoded_str[1024];
        int len;
        //strcpy(str, "Aladdin:open sesame");
        encoded_str[0] = 0;
        decoded_str[0] = 0;

        for(int k=0;k<50;k++)
        {
                sprintf(str,"%s%d",str,k);
                memset(encoded_str,0,sizeof(encoded_str));
                memset(decoded_str,0,sizeof(decoded_str));
                base64_encode_padding(encoded_str, str,strlen(str));
                len=base64_decode_padding(decoded_str, encoded_str);
                printf("Original: %s\n", str);
                printf("Encoded: %s\n", encoded_str);
                printf("Decoded(%d): ",len);
                for(int i=0;i<len;i++)
                        printf("%c ",decoded_str[i]);
                printf("\n\n");
        }
        return 0;
}
#endif