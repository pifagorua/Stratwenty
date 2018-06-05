#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H

#include <include/depends.h>
#include <include/actor2d.h>

struct Action {
    int id;
    QVariant data;
    vec2 transform_data[2];

    Action() :
        id(0), data(0), transform_data({vec2(0, 0), vec2(0, 0)}) {}
    Action(int i, QVariant d) : id(i)
    {
        data.setValue(d);
    }
    Action(int i, QVariant d, vec2 da, vec2 dd)
        : id(i)
    {
        data.setValue(d);
        transform_data[0] = da;
        transform_data[1] = dd;
    }

    void reset()
    {
        id = 0;
        data.clear();
    }

    QString stringData()
    {
        if(data.userType() == QMetaType::QString)
            return data.toString();
        return "";
    }

    bool boolData()
    {
        if(data.userType() == QMetaType::Bool)
            return data.toBool();
        return 0;
    }

    vec2 vec2Data(int i)
    {
        return transform_data[i];
    }
};

class GameEvents
{
public:
    GameEvents();

    vec2 mousePos();
    bool isMouseOver(SDL_Rect *rect);
    bool isMouseDown(int mbtn);
    bool isMouseUp(int mbtn);

    Action processUIobject(Actor2d &obj);
private:
    QMap<QString, QString> ui_btns;
};

#endif // GAMEEVENTS_H
