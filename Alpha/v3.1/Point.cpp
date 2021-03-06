#ifndef HEADER_HPP
#include "Header.hpp"
#endif

Point::Point(short int x, short int y)
{
  X = x;
  Y = y;
}

Point::Point()
{
  X = 0;
  Y = 0;
}

//copy constructor
Point::Point(const Point& rhs)
{
  X = rhs.X;
  Y = rhs.Y;
}

Point& Point::operator=(const Point& rhs)
{
  if(this != &rhs){
    X = rhs.X;
    Y = rhs.Y;
  }
  return *this;
}
