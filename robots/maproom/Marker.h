#define MOVING_TIME 10 // sung to the tune of "Closing Time"

class Marker {
  int position; // 0 = up, 1 = down
  int movingCount;

  public:
  Marker() {}
  Marker(int pos) {
    position = pos;
    movingCount = 0;
  }

  int setPosition(int pos) {
    if (pos == position) return pos;
    // todo: add servo code
    movingCount++;
    if (movingCount > MOVING_TIME) {
      movingCount == 0;
      position = pos;
    }
    return position;
  }

  int getPosition() {
    return position;
  }
};
