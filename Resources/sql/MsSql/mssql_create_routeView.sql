DROP VIEW if exists RouteView;
CREATE VIEW RouteView (Route, AlphaRoute, RouteName, SegmentId, Description,
                                     StartDate, EndDate, CompanyKey, CompanyName, TractionType, Tracks,
                                     OneWay, TrackUsage, Length, Direction, Type, Street, Location,
                                     StartLat, StartLon, EndLat, EndLon, TTKey, Next, Prev, NextR, PrevR,
                                     NormalEnter, NormalLeave, ReverseEnter, ReverseLeave,
                                     Sequence, ReturnSeq, Points, PointArray, BaseRoute, DoubleDate,
                                     SegmentStartDate, SegmentEndDate)
AS Select a.route, b.routeAlpha, name, s.segmentId, s.description, a.startDate, a.endDate,
       a.companyKey, c.description, t.description, s.tracks, a.OneWay, a.TrackUsage, s.length, s.direction,
       s.Type, s.street, s.location, s.startLat, s.startLon, s.endLat, s.endLon, t.tractionType,
       a.Next, a.Prev, a.nextR, a.prevR, a.NormalEnter, a.NormalLeave, a.ReverseEnter, a.ReverseLeave,
       a.Sequence, a.ReverseSeq, points, pointArray, b.BaseRoute, s.DoubleDate, s.startDate, s.endDate
       from Routes a
       join Segments s on a.linekey = s.segmentId
       join AltRoute b on a.route = b.route
       join TractionTypes t on a.tractionType = t.tractionType
       join Companies c on a.companyKey = c.[key];

