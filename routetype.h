#ifndef ROUTETYPE_H
#define ROUTETYPE_H
enum RouteType { Surface, SurfacePRW, RapidTransit, Subway, Rail,
                 Incline, MagLev, Elevated, Other };
static bool to_enum(int n)
{
    switch (n)
    {
    case Surface:
    case SurfacePRW:
    case RapidTransit:
    case Subway:
    case Rail:
    case Incline:
    case MagLev:
    case Elevated:
    case Other:
        return true;
    default:
        return false;
    }
}

#endif // ROUTETYPE_H
