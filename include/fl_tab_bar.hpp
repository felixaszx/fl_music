#ifndef FL_TAB_BAR_HPP
#define FL_TAB_BAR_HPP

#include "fl_ext.hpp"
#include <algorithm>

namespace fl
{
    struct Tab : public fle::Ext<fl::RadioButton>
    {
        bool removable_ = true;
        bool hover_ = false;
        Fl_Boxtype close_box_{};
        Fl_Color close_color_{};

        Tab(int w, int h, const char* label);

        virtual void draw() override;
        virtual int handle(int event) override;
    };

    template <typename TabT = Tab>
    struct TabBar : public fle::Ext<fl::Scroll>
    {
        TabBar(int x, int y, int w, int h);

        virtual int handle(int event) override;
        virtual void draw() override;
        TabT* add_tab(int width, const char* label);
        Tab* get_tab(int index) { return static_cast<TabT*>(child(index + 2)); }
    };

    // template implementations

    template <typename TabT>
    inline TabT* TabBar<TabT>::add_tab(int width, const char* label)
    {
        TabT* tmp_tab = new TabT(width, h(), label);
        add(tmp_tab);
        int x_offset_ = 0;
        for (int i = 0; i < children(); i++)
        {
            if (child(i) == &hscrollbar || child(i) == &scrollbar)
            {
                continue;
            }

            child(i)->position(x_offset_, y());
            x_offset_ += child(i)->w();
        }
        return tmp_tab;
    }

    template <typename TabT>
    inline void TabBar<TabT>::draw()
    {
        fle::Ext<fl::Scroll>::draw();
    }

    template <typename TabT>
    inline int TabBar<TabT>::handle(int event)
    {
        if (event == FL_MOUSEWHEEL)
        {
            if (hscrollbar.visible())
            {
                int target = std::clamp(xposition() + 5 * Fl::event_dy(),       //
                                        static_cast<int>(hscrollbar.minimum()), //
                                        static_cast<int>(hscrollbar.maximum()));
                scroll_to(target, 0);
            }
        }

        return fle::Ext<fl::Scroll>::handle(event);
    }

    template <typename TabT>
    inline TabBar<TabT>::TabBar(int x, int y, int w, int h)
        : fle::Ext<fl::Scroll>(x, y, w, h)
    {
        end();
        type(HORIZONTAL);
        hscrollbar.box(FL_FLAT_BOX);
        hscrollbar.slider(FL_FLAT_BOX);
        hscrollbar.color(fle::Color(0xF0F0F0));
        hscrollbar.color2(fle::Color(0xCDCDCD));
        hscrollbar.labelcolor(fle::Color(0x606060));
    }

#endif // FL_TAB_BAR_HPP
};
