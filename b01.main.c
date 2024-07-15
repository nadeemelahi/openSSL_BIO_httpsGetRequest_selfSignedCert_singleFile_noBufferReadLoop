/*
 * author: Nadeem Elahi
 * nadeem.elahi@gmail.com
 * nad@3deem.com
 * license: gpl v3
 */


/*
 * HOW TO
 *
 * Must make a self signed cert:
 *
 * TrustStore.pem
 *
 * $openssl req -new -x509 -days 365 -noenc 
 *	    -out ./TrustStore.pem 
 *	    -keyout ./local.key
 *	    
 * Compiling:
 *
 * gcc -lssl -lcrypto b01.main.c
 *
 *
 * Running:
 *
 * ./a.out 3deem.com /
 * 
 * or
 *
 * ./a.out 3deem.com /html5/js-templating/index.html
 *
 * No buffer read looping included, just does 1 read of max 1024 MB
 *
 */
#define LIM 1024

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# include  "openssl/bio.h"
# include  "openssl/ssl.h"


int main ( int argc , char * argv [ ] ) {

	if ( argc != 3 ){
		fprintf( stderr , "USAGE error\n");
		fprintf( stderr , "     $./a.out 3deem.com / \n");
		fprintf( stderr , "     $./a.out 3deem.com /webgl \n");
		exit(1);
	}

	// https://developer.ibm.com/tutorials/l-openssl/
	printf ( "main()\n" ) ;
	int ret;

	SSL_load_error_strings();
	//ERR_load_BIO_strings(); // warning: depreciated since OpenSSL 3.0
	// not needed anymore
	OpenSSL_add_all_algorithms();
	printf ( "initialized openssl() \n" );
	
	// find out version of your systems
	// $openssl version
	
        SSL_CTX *ctx;
        ctx = SSL_CTX_new( SSLv23_client_method() );

	// etc/ssl/certs
	// create certs
	// $openssl req -new -x509 -days 365 -noenc 
	//    -out ./TrustStore.pem 
	//    -keyout ./local.key
	
	ret = SSL_CTX_load_verify_locations(
			ctx, 
			"./TrustStore.pem", 
			NULL
			);

	if( !ret ) {
		fprintf ( stderr , "SSL_CTX_load_verify_locations() error" );
		exit ( 1 ) ;
	}

	printf("SSL_CTX_load_verify_locations() CHECK\n");



	BIO *bio;
        bio = BIO_new_ssl_connect ( ctx ) ;
	printf("BIO_new_ssl_connect () CHECK\n");


	SSL *ssl;

	BIO_get_ssl ( bio , ssl ) ;
	printf("BIO_get_ssl () CHECK\n");


	
	/*
	ret = SSL_get_verify_result( ssl ) ;
	printf("ret: %d\n" , ret );

	printf("x509_V_OK: %d\n", X509_V_OK); // 0
	if( ret != X509_V_OK) {
		fprintf ( stderr , "SSL_get_verify_result() error" );
		exit ( 1 ) ;
	}
	// does not work on debian bookworm
	*/
	

	//SSL_set_mode ( ssl , SSL_MODE_AUTO_RETRY ) ;
	//printf("SSL_set_mode() CHECK\n");
	// does not work on debian bookworm

		

	char hostNportBuf[200] = {0};
	char *hostNport = hostNportBuf;

	sprintf( hostNport , "%s:443" , argv[1] ) ;
		
	BIO_set_conn_hostname ( bio , hostNport ) ;
	printf("BIO_set_conn_hostname() CHECK\n");


	ret = BIO_do_connect ( bio ) ;
	if ( ret < 1 ) {
		fprintf ( stderr , "BIO_do_connect() error" );
		exit ( 1 ) ;
	}

	printf("BIO_do_connect () CHECK\n");





	char getBuf[200] = {0};
	char *get = getBuf;

	sprintf ( 
			get , 
			"GET %s HTTP/1.1\r\nHost: %s\r\n\r\n" , 
			argv[2] ,
			argv[1] 
		);

	ret =  BIO_write ( bio , get , strlen ( get ) ); 

	if ( ret < 1 ) {
		fprintf ( stderr , "BIO_write() error" );
		exit ( 1 ) ;
	}

	printf ( "BIO_write() CHECK\n" );


	char respBuf[LIM] = {0};
	char *resp = respBuf;

	ret = BIO_read ( bio , resp , LIM );

	if ( ret == 0 ) {

		fprintf ( stderr , "BIO_read() error - closed connection" );
		exit ( 1 ) ;
	} else if ( ret < 0 ) {

		fprintf ( stderr , "BIO_read() error - failed to read" );
		exit ( 1 ) ;
	}

	printf ( "BIO_read() CHECK\n" );


	printf( "\n%s\n\n\n" , resp ) ;



	// finally closing the connection
	SSL_CTX_free ( ctx ) ;
	// BIO_reset ( bio ) ; // if want to re-use for a second connection
	BIO_free_all ( bio ) ;


	return 0;


	// $man BIO_new_ssl_connect
	// client server example programs
}
