#ifndef RC_H
#define RC_H

#define BUFLEN 1000
#define MAXSPACE  12000

typedef void (*rccallback)(unsigned long val, int  numbits, uint8_t prot);

/*
 * Interrupt variables
 */
volatile uint8_t RXpin;
volatile int buf[BUFLEN];
volatile unsigned long puls = micros();
volatile uint16_t wIndex = 0;
//volatile unsigned long timeout = 0;
//volatile unsigned long cycles = MAXSPACE*80;

/*
 * Interrupt functions
 */
//void timer_ISR() {
  //puls = micros();
  //buf[wIndex++] = MAXSPACE & -2;
  //wIndex %= BUFLEN;
//}

void pin_ISR() {
  unsigned long val = (micros() - puls);
  puls = micros();
  /*
  if (val > MAXSPACE) val = MAXSPACE;
  if (digitalRead(RXpin)) val &= -2;
  else val |= 1;
  buf[wIndex++] = val;
  */
  buf[wIndex++] = (digitalRead(RXpin)) ? (val &= -2) : (val |= 1);
  wIndex %= BUFLEN;
//  timer1_write(cycles);
}

/*
 * Structures, Variables and constants
 */
struct INTPAIR {
  int high;
  int low;
};

struct PROT {
  INTPAIR sync;
  INTPAIR ones;
  INTPAIR zeros;
  uint8_t len;
};
PROT pr[] {
  { { 335, 10900}, {1144, 260}, {404, 1000}, 24}, //  BT 24
  { { 407,  7172}, {1412, 582}, {422, 1552}, 32}, //  HN 32
  { { 290,  2345}, {1050, 450}, {290, 1225}, 24}, //  HN 24
};
uint16_t prLen = sizeof(pr) / sizeof(PROT);
uint8_t TXpin;
uint16_t rIndex = 0;
unsigned long codevalue;
uint16_t numbits = 0;
int prot = 0;
INTPAIR bits = {0, 0};
INTPAIR stopbit = {500, 50};

/*
 * Private functions
 */
bool diff(int a, int b) {
  return abs(a - b) < round(a * .15);
}

void getProt(INTPAIR bits) {
  numbits = 0;
  codevalue = 0UL;
  prot = 0;
  for (int i = 0; i < prLen; i++) {
    if (diff(pr[i].sync.low, bits.low)) {
      if (diff(pr[i].sync.high, bits.high))  {
        prot = i + 1;
      }
    }
  }
}

int validbit(INTPAIR bits) {
  if (bits.low == MAXSPACE) bits.low = pr[prot - 1].zeros.low;
  int vbit = (bits.high > (bits.low));  //  OBS! not good - check for length of high
  //int vbit = !diff(pr[prot-1].zeros.low,bits.low);
  INTPAIR * x = vbit ? &pr[prot - 1].ones : &pr[prot - 1].zeros;
  if (!diff( x->high + x->low, bits.high + bits.low)) return -1;
  return vbit;
}

void sendBit(INTPAIR * p) {
  digitalWrite(TXpin, HIGH);
  delayMicroseconds(p->high + 50);
  digitalWrite(TXpin, LOW);
  delayMicroseconds(p->low - 50);
}

rccallback gotData;
/*
 * Public functions
 */
void rconData(rccallback _cb) {   //  Valid data callback
  gotData = _cb;
}

void rcbegin(uint8_t _RXpin, uint8_t _TXpin) {
  RXpin = _RXpin;
  TXpin = _TXpin;
//  timeout = 0;
  pinMode(RXpin, INPUT );
  pinMode(TXpin, OUTPUT );
  digitalWrite(TXpin, LOW);
  noInterrupts();
  attachInterrupt(digitalPinToInterrupt(RXpin), pin_ISR, CHANGE);
//  timer1_isr_init();
//  timer1_attachInterrupt(timer_ISR);
//  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  interrupts();
}

void rcloop() {
  noInterrupts();
  bool bufempty = (rIndex == wIndex);
  interrupts();
  if (bufempty) return;
  if (buf[rIndex] & 1) bits = {buf[rIndex] & -2, 0};
  else bits.low = buf[rIndex];
  ++rIndex %= BUFLEN;
  if (!bits.low) return;  //  not a valid pair?
  if (prot) {   //  is protocol defined?
    int thisbit = validbit(bits);
    if (thisbit >= 0) {  //  if not valid, we may have end of frame
      codevalue = (codevalue << 1) + thisbit;
      if (++numbits == pr[prot - 1].len) {
        if (gotData) gotData(codevalue, numbits, prot);
        prot = 0;
      }
    } else prot = 0;
  }
  if (!prot) getProt(bits);
  bits = {0, 0};
}

void rcsend(uint8_t prot, uint32_t value) {
  noInterrupts();
  INTPAIR * p;
  for (int y = 0; y < 4; y++) {
    p = &pr[prot - 1].sync;
    sendBit(p);
    for (int i = pr[prot - 1].len - 1; i >= 0; i--) {
      if ((value >> i) & 1) p = &pr[prot - 1].ones;
      else p = &pr[prot - 1].zeros;
      sendBit(p);
    };
  }
  sendBit(&stopbit);
  interrupts();
}

#endif

