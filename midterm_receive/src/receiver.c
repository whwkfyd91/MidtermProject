
#include "pbm.h"
#include "rtp.h"
#define DATA_SIZE 1000000 //We assume not more than this number of bytes will be recieved

void getSum(int start, int end, unsigned char *data, int* des);
unsigned char* readData(unsigned char *data, unsigned long length, struct image_header *ih);

int main(int argc, const char * argv[]) {
    int                sock;
    struct sockaddr_in local;
    int                timeout;
    unsigned long number_of_recv_bytes;
    unsigned char *data, *yuv420, *ycbcr;
    struct image_header ih;
    char con_rgbImg[] = "./received_img.pbm";
    
    if (argc != 2) {
        printf("Usage: %s <port>", argv[0]);
        exit(1);
    }
    data = (unsigned char *)malloc(DATA_SIZE);

    // create new socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock >= 0) {
        // prepare local address
        memset(&local, 0, sizeof(local));
        local.sin_family      = AF_INET;
        local.sin_port        = htons(atoi(argv[1]));
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        
        // bind to local address
        if (bind(sock, (struct sockaddr *)&local, sizeof(local)) == 0) {
            // set recv timeout
            timeout = RTP_RECV_TIMEOUT;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
            
            //recieve data here;
            number_of_recv_bytes = rtp_recv_packets(sock, data);
        }
        close(sock);
    }

    /* step1. Read Data(received data.. YUV420 format)*/
    yuv420 = readData(data, number_of_recv_bytes, &ih);
    /* step2. Convert YUV420 to YCbCr.*/
    ycbcr = getYUV420toYCbCr(yuv420, ih);
    /* step3. Convert YCbCr to RGB*/
    getYCbCrtoRGB(con_rgbImg, ycbcr, ih); //make file 'received_img.pbm'

    free(data);
    free(yuv420);
    free(ycbcr);
    return 0;
}

//Add data(start, end) to des-address.
void getSum(int start, int end, unsigned char *data, int* des){
	int i=start, j, k, sum=0, tmp=1;

	for(k=end-start;k>=1;k--){
		for(j=0;j<k-1;j++){
			tmp *=10;
		}
		sum += (data[i++]-48)*tmp;
		tmp=1;
	}
	*des = sum;
}

//get img_header and get payload.
unsigned char* readData(unsigned char *data, unsigned long length, struct image_header *ih){
	int i=3,k;
	unsigned char* yuv420;

	ih->format[0] = data[0];
	ih->format[1] = data[1];
	ih->format[2] = '\0';

	while(data[i]!= 0x20)	i++;
	getSum(3,i,data,&ih->rows);

	k = ++i;
	while(data[i]!= 0x20)	i++;
	getSum(k,i,data,&ih->cols);

	k = ++i;
	while(data[i]!= 0x20)	i++;
	getSum(k,i++,data,&ih->levels);

	printf("[receiver]received_img header: %d %d %d, size: %d\n", ih->cols, ih->rows, ih->levels, length-i);
	yuv420 = (unsigned char*)malloc(1000000);
	memcpy(yuv420, data+i, length-15);
	printf("[receiver]received_Img_length : %d\n", length);

	return yuv420;
}

