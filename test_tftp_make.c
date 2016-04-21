#include "tftp.h"

int main() {
	
	char ack[TFTP_SIZE];
	size_t length;
	if(tftp_make_ack(ack, &length, 5) < 0) {
		fprintf(stderr, "Error ack creation");
		exit(EXIT_FAILURE);
	}

	printf("ACK = %d\t%d\n", extract_type(ack), extract_blocknumber(ack));
	
	char rrq[TFTP_SIZE];
	const char *file = "text.txt";
	tftp_make_rrq(rrq, &length, file);
	
	printf("RRQ = %d\t%s\t%s\n", extract_type(rrq),extract_file(rrq),extract_mode(rrq, 3 + sizeof(extract_file(rrq))));
	
	char error[TFTP_SIZE];
	const char *message = "Fichier test.txt pas trouvé";
	if(tftp_make_error(error, &length, FILE_NOT_FOUND, message) < 0) {
		fprintf(stderr, "Error error creation");
		exit(EXIT_FAILURE);
	}
	
	printf("%d\n", strlen(message));
	printf("ERROR = %d\t%d\t%s\n", extract_type(error), extract_errcode(error), extract_err_msg(error));
	
	
	
	return EXIT_SUCCESS;
}
