#ifndef TRANSL_H
#define TRANSL_H

/*
 * Convert betwine rc433 codes and topic/value
 */
 
class TOPIC {
  private:
  public:
    String plain = "";
    String cmd = "";
    void set(char * _topic) {
      plain = _topic;
      int pos = plain.lastIndexOf("/");
      cmd = plain.substring(pos);
      if ((cmd == "/state") or (cmd == "/cmd")) {
        plain = plain.substring(0, pos);
      } else {
        cmd = "";
      }
    }
} topicconverter;

struct CODE {
  unsigned long code;
  int chan;
  int state;
};
CODE trlist[] = {
  {9130048, 0, 1}, {9225328, 0, 1}, {8790960, 0, 1}, {9348912, 0, 1},//HN1 on
  {8993168, 0, 0}, {8751264, 0, 0}, {9258272, 0, 0}, {8446480, 0, 0},//HN1off
  {9130052, 1, 1}, {9348916, 1, 1}, {8790964, 1, 1}, {9225332, 1, 1},//HN2on
  {9258276, 1, 0}, {8751268, 1, 0}, {8993172, 1, 0}, {8446484, 1, 0},//HN2off
  {8790972, 2, 1}, {9225340, 2, 1}, {9130060, 2, 1}, {9348924, 2, 1},//HN3on
  {8751276, 2, 0}, {8993180, 2, 0}, {8446492, 2, 0}, {9258284, 2, 0},//HN3off
  {9258274, 3, 1}, {8446482, 3, 1}, {8993170, 3, 1}, {8751266, 3, 1},//HN4on
  {9348914, 3, 0}, {9130050, 3, 0}, {9225330, 3, 0}, {8790962, 3, 0},//HN4off
  {8446494, 4, 1}, {9258286, 4, 1}, {8751278, 4, 1}, {8993182, 4, 1},//HN5on
  {9130062, 4, 0}, {9348926, 4, 0}, {8790974, 4, 0}, {9225342, 4, 0},//HN5off
  {1121188563, 5, 1}, {4164119955, 5, 1},//HNAon
  {813340819, 5, 0}, {1233086675, 5, 0},//HNAoff
  {2402745557, 6, 1}, {1497335701, 6, 1},//HNBon
  {2737501909, 6, 0}, {2196402069, 6, 0},//HNBoff
  {956148187, 7, 1}, {2938304923, 7, 1},//HNCon
  {2635276507, 7, 0}, {1451000219, 7, 0},//HNCff
  {1205969520, 8, 1}, {2335742528, 8, 1},//HNDon
  {1364462960, 8, 0}, {3606038592, 8, 0},//HNDoff
  {348364945, 9, 1}, {1106750369, 9, 1},//HNEon
  {202680977, 9, 0}, {3684437921, 9, 0},//HNEoff

  { 83029, 10, 1}, { 83028, 10, 0}, //  BT1
  { 86101, 11, 1}, { 86100, 11, 0}, //  BT2
  { 70741, 12, 1}, { 70740, 12, 0}, //  BT3
  { 21589, 13, 1}, { 21588, 13, 0}, //  BT4
  { 83029, 10, 1}, { 83028, 10, 0}, //  BT1
  { 86101, 11, 1}, { 86100, 11, 0}, //  BT2
  { 70741, 12, 1}, { 70740, 12, 0}, //  BT3
  { 21589, 13, 1}, { 21588, 13, 0}, //  BT4
  { 83029, 10, 1}, { 83028, 10, 0}, //  BT1
  { 86101, 11, 1}, { 86100, 11, 0}, //  BT2
  { 70741, 12, 1}, { 70740, 12, 0}, //  BT3
  { 21589, 13, 1}, { 21588, 13, 0}, //  BT4
};

struct CH {
  int prot;
  unsigned int bits;
  String topic;
};
CH ch[] = {
  { 3, 24, "esp8266/HN1"},
  { 3, 24, "esp8266/HN2"},
  { 3, 24, "esp8266/HN3"},
  { 3, 24, "esp8266/HN4"},
  { 3, 24, "esp8266/HN5"},
  { 2, 32, "esp8266/HNA"},
  { 2, 32, "esp8266/HNB"},
  { 2, 32, "esp8266/HNC"},
  { 2, 32, "esp8266/HND"},
  { 2, 32, "esp8266/HNE"},
  { 1, 24, "esp8266/BT1"},
  { 1, 24, "esp8266/BT2"},
  { 1, 24, "esp8266/BT3"},
  { 1, 24, "esp8266/BT4"},
};
/*
struct CODERETURN {
  unsigned long code;
  String topic;
  int state;
};
*/
class TRANSLATE {
  private:
    bool isCmd(char * s) {
      return isCmd(s, "/cmd");
    };
    bool isCmd(char * s, char * cmp) {
      int x = ((String)s).lastIndexOf("/");
      String a1 = ((String)s).substring(x);
      return !strcmp(&a1[0], &cmp[0]);
    };
    bool port2code(int port, bool state) {
      for (int i = 0; i < (sizeof(trlist) / sizeof(CODE)); i++) {
        if ((trlist[i].chan == port) && (trlist[i].state == state)) {
          prot = ch[port].prot;
          topic = ch[port].topic;
          code = trlist[i].code;
          bits = ch[port].bits;
          return true;
        }
      }
      prot = 0;
      topic = "";
      code = 0;
      bits = 0;
      return false;
    }
  public:
    unsigned long code;
    String topic = "";
    int state = 0;
    int prot = 0;
    int bits = 0;
    bool decode(unsigned long code) {
      topic = "";
      state = 0;
      for (int i = 0; i < sizeof(trlist) / sizeof(CODE); i++) {
        if (code == trlist[i].code) {
          topic = ch[trlist[i].chan].topic;
          state = trlist[i].state;
          return true;
        }
      }
      return false;
    }
    bool encode(String xtopic, bool xstate) {
      topic = xtopic;
      state = xstate;
      for (int i = 0; i < (sizeof(ch) / sizeof(CH)); i++) {
        if (xtopic == ch[i].topic) {
          return port2code(i, xstate);
        }
      }
      topic = "";
      state = false;
      return false;
    }
};

#endif

