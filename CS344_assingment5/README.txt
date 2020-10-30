Name: Tseng, Yu-Wen
Course: CS344
Assignment4-OTP

Implementation of a One Time Pad (OTP)

Compileall:
gcc -o otp_enc otp_enc.c
gcc -o otp_enc_d otp_enc_d.c
gcc -o otp_dec otp_dec.c
gcc -o otp_dec_d otp_dec_d.c
gcc -o keygen keygen.c


It consists of 5 small programs:
*) otp_enc_d: Performs the actual encoding. Runs in the background as a daemon. Listens on a particular port for the plaintext and the key. When a connection is made to otp_enc, it forks off a seperate process immediately so that it can handle concurrent connections. 
                  
              
*) otp_enc: Connects to otp_enc_d and asks it to perform a onetime pad style encryption as above. When it receives the encrypted text, it outputs to stdout.
                 
              
*) otp_dec_d: Performs exactly like otp_enc_d except that it decrpyts the ciphertext given to it.
                  

*) otp_dec: Performs excatly like otp_enc except that it asks otp_dec_d to decrypt ciphertext using a passed in ciphertet and key.
                  
                  
*) keygen: Generates a randomized key of specified length using the 26 capital letters and the space character.
                 