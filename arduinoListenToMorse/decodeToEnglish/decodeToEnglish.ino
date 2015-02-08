// use a microphone to listen for morse code, then decypher the message.
#define THRESHOLD    (44)
#define MAX_SAMPLES  (5)

#define BAUD         (100.0)
#define WAIT         (5.0)
#define AVG_LONG     (BAUD*3.0/WAIT)
#define AVG_SHORT    (BAUD*1.0/WAIT)
#define AVG_NEWWORD  (BAUD*7.0/WAIT)
#define MINIMUM      (AVG_SHORT/4.0)

#define MAX_PATTERN  (64)

#define NUM_CODES    (54)

//                            0        10        20        30        40        50
//                            0123456789012345678901234567890123456789012345678901234
static const char *letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,?'!/()&:;=+-_\"$@";

// each code represents a character.
// 1 represents a dot and 0 represents a dash.
// the order of these codes matches the order of the characters in *letters.
static const char *codes[NUM_CODES] = {
  "10",      // A, codes[0]
  "0111",    // B, codes[1]
  "0101",    // C
  "011",     // D
  "1",       // E
  "1101",    // F
  "001",     // G
  "1111",    // H
  "11",      // I
  "1000",    // J
  "010",     // K
  "1011",    // L
  "00",      // M
  "01",      // N
  "000",     // O
  "1001",    // P
  "0010",    // Q
  "101",     // R
  "111",     // S
  "0",       // T
  "110",     // U
  "1110",    // V
  "100",     // w
  "0110",    // x
  "0100",    // y
  "0011",    // z
  "00000",   // 0
  "10000",   // 1
  "11000",   // 2
  "11100",   // 3
  "11110",   // 4
  "11111",   // 5
  "01111",   // 6
  "00111",   // 7
  "00011",   // 8
  "00001",   // 9
  "101010",  // .
  "001100",  // ,
  "110011",  // ?
  "100001",  // '
  "010100",  // !
  "01101",   // /
  "01001",   // (
  "010010",  // )
  "10111",   // &
  "000111",  // :
  "010101",  // ;
  "01110",   // =
  "10101",   // +
  "01110",   // -
  "110010",  // _
  "101101",  // "
  "1110110", // $
  "100101",  // @, codes[54]
};


int top=0;

int samples[MAX_SAMPLES];
int si=0;
int mi=0;
int total=0;

int c=0;
int is_on=0;


char pattern[MAX_PATTERN];
int pi=0;


void setup() {
  Serial.begin(57600);
  
  for(int i=0;i<MAX_SAMPLES;++i) {
    samples[i]=0;
  }
  for(int i=0;i<MAX_PATTERN;++i) {
    pattern[i]=0;
  }
}


void loop() {
  int volume=analogRead(0);
  
  total -= samples[si];
  samples[si] = volume;
  total += samples[si];
  if( mi < MAX_SAMPLES ) mi++;
  si = (si+1) % MAX_SAMPLES;
  int average = total / mi;
  
  if( top < average ) top = average;
  
  
  int x = 10.0 * (float)(average-THRESHOLD)/(float)(top-THRESHOLD);
  if(x<0) x=0;
  if(x>10) x=10;
  
  if(x>1) {
    // noise!
    if(is_on==0) {
      // noise has just started.
      if( c > MINIMUM ) {
        
        // Was the silence a new word or a new letter?
        if( c > (AVG_NEWWORD+AVG_SHORT)/2.0 ) {
          pattern[pi]=0;
          findLetter();
          // new word, extra \n
          Serial.println();
          // start counting - and . all over again.
          pi=0;
        } else if( c > (AVG_LONG+AVG_SHORT)/2.0 ) {
          pattern[pi]=0;
          findLetter();
          // start counting - and . all over again.
          pi=0;
        }

      }
      // remember noise started
      is_on=1;
      c=0;
    }
  } else {
    // silence!
    if(is_on==1) {
      // silence is new
      if( c > MINIMUM ) {
        // Was the noise a long or a short?
        if( c > (AVG_LONG + AVG_SHORT)/2.0 ) {
          Serial.print('-');
          pattern[pi++]='0';
        } else {
          Serial.print('.');
          pattern[pi++]='1';
        }
      }
      // remember silence started
      is_on=0;
      c=0;
    }
  }

  c++;
  
  delay(WAIT);
}


// pattern contains the received longs and shorts,
// saved as 1s and 0s.  Find the matching code in the list
// then find the matching printable character.
// print '?' if nothing is found
void findLetter() {
  int i,j;
  
  // go through all the codes  
  for(i=0;i<NUM_CODES;i++) {
    // check if code[i] matches pattern exactly.
    if(strlen(pattern) == strlen(codes[i]) && strcmp(pattern,codes[i])==0) {
      // match!
      Serial.print(' ');
      Serial.println(letters[i]);
      return;
    }
  }
  Serial.print('?');
}
