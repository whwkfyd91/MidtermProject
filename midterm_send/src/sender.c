 
#include "pbm.h"
#include "rtp.h"
#define DATA_SIZE 1000000
#define PAYLOAD_SIZE 1000

int addAddr(int length, int num,unsigned char *addr);
void getResult(int d_start, int s_end, unsigned char* des, unsigned char* src);

int main(int argc, const char * argv[]){
    int                 sock, i,k,j;
    struct sockaddr_in  local;
    struct sockaddr_in  to;
    u_int32             rtp_stream_address;

    struct image_header ih;
    char ori_rgbImg[] = "./start.pbm";		//origin file name
    char ycbcr_img[] = "./start_yuv.pbm";	//ycbcr file name
    unsigned long       data_size;
    unsigned char       *ycbcr, *result;
    char *data,*yuv420;
    
    if (argc != 3) {
        printf("\nUsage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    /* step1. read Image.*/
	data = readImage(ori_rgbImg, &data_size, &ih);
	/* step2. convert RGB to YCbCr.*/
	ycbcr = getRGBtoYCbCr(ycbcr_img,data, ih);
	/* step3. convert YCbCr to YUV420.*/
	yuv420 = getYCbCrtoYUV420(ycbcr, ih);

	result = (unsigned char*)malloc(DATA_SIZE);
	memcpy((char*)result, &ih.format,3);
	result[2] = 0x20;

	/* Result-data add num(rows, cols, levels)*/
	i = addAddr(3,ih.rows,result);
	i = addAddr(i,ih.cols,result);
	i = addAddr(i,ih.levels,result);

	/* Result-data add payload(yuv420 format)*/
    getResult(i, (data_size/2), result, yuv420);

    // initialize RTP stream address */
    rtp_stream_address = inet_addr(argv[1]);
    
    // if we got a valid RTP stream address... */
    if (rtp_stream_address != 0) {
        sock = socket(AF_INET, SOCK_DGRAM, 0); // create new socket for sending datagrams
        if (sock >= 0) {
            // prepare local address
            memset(&local, 0, sizeof(local));
            local.sin_family      = AF_INET;
            local.sin_port        = htons(INADDR_ANY);
            local.sin_addr.s_addr = htonl(INADDR_ANY);
            // bind to local address
            if (bind(sock, (struct sockaddr *)&local, sizeof(local)) == 0) {
                // prepare RTP stream address
                memset(&to, 0, sizeof(to));
                to.sin_family      = AF_INET;
                to.sin_port        = htons(atoi(argv[2]));
                to.sin_addr.s_addr = rtp_stream_address;

                // send RTP packets(Result-data)
                rtp_send_packets( sock, &to, result, (data_size/2)+i, PAYLOAD_SIZE);
            }
            close(sock);
        }
    }
    printf("[sender]sending Img_szie/2+i = %d\n", (data_size/2)+i);

    free(ycbcr);
    free(yuv420);
    free(data);
    return 0;
}

/* Result-data should add num(cols, rows,levels)*/
int addAddr(int length, int num, unsigned char *addr){
    int su=1,tmp=1,i,cal,check=10;

    while(num / check)
	{
		su++;
		check *= 10;
	}

    for(i=0;i<su-1;i++)
    	tmp *=10;

    for(i=length;i<length+su;i++){
    	cal = num/tmp;
    	addr[i] = 48+cal;
    	num = num%tmp;
    	tmp /= 10;
    }
    addr[i++]= 0x20;
    return i;
}

/* Result-data should add payload */
void getResult(int d_start, int s_end, unsigned char* des, unsigned char* src){
	int i;
	for(i=0;i<s_end;i++)
		des[d_start++] = src[i];
}
