#ifndef __PNL_ORIENTATION_H_
#define __PNL_ORIENTATION_H_

#include <string>

namespace naja { namespace PNL {

class PNLOrientation {
  public:
    // class Type {
    //   public:
        enum TypeEnum {
          R0,     // Represents no change in orientation
          R90,    // Represents a 90 degree rotation
          R180,   // Represents a 180 degree rotation
          R270,   // Represents a 270 degree rotation
          MY,     // Represents mirroring about the Y axis
          MYR90,  // Represents mirroring about the Y axis then a 90 degree rotation
          MX,     // Represents mirroring about the X axis
          MXR90   // Represents mirroring about the X axis then a 90 degree rotation
        };
    //     Type(const TypeEnum& typeEnum);
    //     Type(const Type&) = default;
    //     Type& operator=(const Type&) = default;
    //     //std::std::string getstd::string() const;

    //   private:
    //     TypeEnum typeEnum_;
    // };

     TypeEnum _type;

        public: PNLOrientation(const TypeEnum& type = R0) : _type(type) {};
        public: PNLOrientation(const PNLOrientation& orientation) : _type(orientation._type) {};
        public: PNLOrientation(const std::string& ) = delete;

        public: PNLOrientation& operator=(const PNLOrientation& orientation);

        public: operator const TypeEnum&() const {return _type;};

        public: const TypeEnum& getType() const {return _type;};

};

}} // namespace PNL // namespace naja

#endif // __PNL_POINT_H_