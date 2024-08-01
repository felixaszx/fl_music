#include "fl_tab_bar.hpp"
#include <iostream>

namespace fl
{
    Tab::Tab(int w, int h, const char* label)
        : fle::Ext<fl::RadioButton>(0, 0, w < h * 3 ? h * 3 : w, h, label)
    {
        align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    }

    void Tab::draw()
    {
        fle::Ext<fl::RadioButton>::draw();
        if ((hover_ || value()) && removable_)
        {
            fl_draw_box(close_box_, x() + (w() - 3 * h() / 4), y() + h() / 4, h() / 2, h() / 2, close_color_);
        }
    }

    int Tab::handle(int event)
    {
        if (event == FL_RELEASE && removable_)
        {
            int x_pos = Fl::event_x();
            int y_pos = Fl::event_y();

            if (x_pos > x() && x_pos < x() + w())
            {
                if (y_pos > y() && y_pos < y() + h())
                {
                    bool x_condition = x_pos > x() + (w() - 3 * h() / 4) && x_pos < x() + (w() - h() / 4);
                    bool y_condition = y_pos > y() + h() / 4 && y_pos < y() + 3 * h() / 4;

                    if ((x_condition && y_condition) || Fl::event_button() == FL_MIDDLE_MOUSE)
                    {
                        fl::Group* p = parent();
                        p->remove(*this);

                        int x_offset_ = 0;
                        for (int i = 0; i < p->children(); i++)
                        {
                            p->child(i)->position(x_offset_, y());
                            x_offset_ += p->child(i)->w();
                        }

                        p->redraw();
                        delete this;
                        return 0;
                    }
                }
            }
        }

        if (event == FL_ENTER)
        {
            hover_ = true;
            redraw();
        }

        if (event == FL_LEAVE)
        {
            hover_ = false;
            redraw();
        }

        return fle::Ext<fl::RadioButton>::handle(event);
    }
}; // namespace fl
