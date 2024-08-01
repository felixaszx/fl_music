#ifndef FL_FLOATING_GROUP_HPP
#define FL_FLOATING_GROUP_HPP

#include <algorithm>
#include "fltk.hpp"

namespace fl
{
    class FloatingGroup : public Group
    {
      private:
        bool title_in_ = false;
        bool resize_in_ = false;
        int prev_x_ = 0;
        int prev_y_ = 0;
        Group control_;

      public:
        Box title_bar_;
        Box resize_box_;

        FloatingGroup(int x, int y, int w, int h, int th, int rw, const char* label = "");

        void draw() override;
        void deep_redraw();
        int handle(int event) override;
    };
}; // namespace fl

#endif // FL_FLOATING_GROUP_HPP
