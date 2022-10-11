#ifndef TURTLEINTERFACE_H
#define TURTLEINTERFACE_H

class TurtleInterface
{
public:
    bool isBusy() const {return m_busy;}

    // mandatory procedures:
    virtual void forward(float value) = 0; // cm
    virtual void backward(float value) = 0; // cm
    virtual void right(float value) = 0; // deg
    virtual void left(float value) = 0; // deg
    virtual void penUp() = 0;
    virtual void penDown() = 0;

    // optional procedures:
    virtual void stop() {}
    virtual void clearScreen() {}
//    virtual void move(float x, float y) {(void)x; (void)y;}
    virtual void setColor(unsigned int rgb) {(void)rgb;}
//    virtual void sound(float freq, float duration) {(void)freq; (void)duration;}
    virtual void arc(float radius, float degrees) {(void)radius;(void)degrees;}

    virtual void setControl(float v, float w) {(void)v; (void)w;}

protected:
    bool m_busy;
};

#endif // TURTLEINTERFACE_H
