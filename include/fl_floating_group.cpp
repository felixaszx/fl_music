#include "fl_floating_group.hpp"

namespace fl
{
    FloatingGroup::FloatingGroup(int x, int y, int w, int h, int th, int rw, const char* label)
        : Group(x, y, w, h),
          title_bar_(x, y, w, th, label),
          resize_box_(x + w - rw, y + h - rw, rw, rw),
          control_(x, y, w, h)
    {
        end();
        box(FL_FLAT_BOX);
        control_.add(title_bar_);
        control_.add(resize_box_);
        control_.resizable(nullptr);
        control_.box(FL_NO_BOX);
        title_bar_.align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_WRAP);
    }

    void FloatingGroup::draw()
    {
        Group::draw();
        draw_label();
    }

    void FloatingGroup::deep_redraw()
    {
        auto p = parent();
        while (p != nullptr)
        {
            p->redraw();
            p = p->parent();
        }
    }

    int FloatingGroup::handle(int event)
    {
        Group::handle(event);
        switch (event)
        {
            case FL_PUSH:
            {
                prev_x_ = Fl::event_x();
                prev_y_ = Fl::event_y();
                bool x_check = prev_x_ >= x() && prev_x_ <= x() + w();
                bool y_check = prev_y_ >= y() && prev_y_ <= y() + title_bar_.h();
                title_in_ = x_check && y_check;
                x_check = prev_x_ >= x() + w() - resize_box_.w() && prev_x_ <= x() + w();
                y_check = prev_y_ >= y() + h() - resize_box_.h() && prev_y_ <= y() + h();
                resize_in_ = x_check && y_check;

                if (fl::Global::event_button() == FL_MIDDLE_MOUSE && title_in_)
                {
                    this->hide();
                }

                break;
            }
            case FL_DRAG:
            {
                int curr_x_ = Fl::event_x();
                int curr_y_ = Fl::event_y();
                if (title_in_)
                {
                    // cap to correct position
                    int x_final = std::clamp(x() + (curr_x_ - prev_x_), //
                                             parent()->x(),             //
                                             parent()->x() + parent()->w() - w());
                    int y_final = std::clamp(y() + (curr_y_ - prev_y_), //
                                             parent()->y(),             //
                                             parent()->y() + parent()->h() - h());

                    position(x_final, y_final);
                    deep_redraw();
                }
                if (resize_in_)
                {
                    int new_w = w() + (curr_x_ - prev_x_);
                    int new_h = h() + (curr_y_ - prev_y_);
                    if (new_w + x() > parent()->x() + parent()->w())
                    {
                        new_w = w();
                    }
                    if (new_h + y() > parent()->y() + parent()->h())
                    {
                        new_h = h();
                    }

                    int final_w = new_w < resize_box_.w() //
                                      ? resize_box_.w()   //
                                      : new_w;
                    int final_h = new_h < resize_box_.h() + title_bar_.h() //
                                      ? resize_box_.h() + title_bar_.h()   //
                                      : new_h;

                    size(final_w, final_h);
                    title_bar_.size(final_w, title_bar_.h());
                    resize_box_.position(x() + final_w - resize_box_.w(), //
                                         y() + final_h - resize_box_.h());
                    deep_redraw();
                }
                prev_x_ = curr_x_;
                prev_y_ = curr_y_;
                break;
            }
        }
        return 1;
    }
}; // namespace fl
