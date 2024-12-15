#include <iostream>
#include <string>
#include <thread>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <codecvt>

#include <portaudio.h>
#include <sndfile.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

#include "fl_ext.hpp"

using namespace std::chrono_literals;
using namespace fle::literals;
fl::Image* down_scaled = nullptr;
fl::Image* down_scaled_bg = nullptr;

struct SNDSound
{
    double vol_db_ = 0;
    double gain_ = 0.0;
    double peak_ = 1.0;
    size_t curr_p_ = 0;
    std::vector<float> buffer_;
    SF_INFO info_{};
};

int stream_callback(const void* input, void* output, unsigned long frame_count,
                    const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* data)
{
    auto sound = (SNDSound*)data;
    auto* out = (float*)output;
    sound->curr_p_ = std::clamp<size_t>(sound->curr_p_, 0, sound->buffer_.size());

    double factor = 1;
    factor = (sound->gain_ + sound->vol_db_) / 20;
    factor = std::pow(10, factor);
    factor = std::min(factor, 1 / sound->peak_);

    for (size_t f = 0; f < frame_count * sound->info_.channels; f++)
    {
        if (sound->curr_p_ >= sound->buffer_.size())
        {
            *out = 0;
            out++;
        }
        else
        {
            *out = factor * sound->buffer_[sound->curr_p_];
            out++;
            sound->curr_p_++;
        }
    }

    return 0;
}

int wmain(int argc, wchar_t** argv)
{
    if (argc <= 1)
    {
        return EXIT_SUCCESS;
    }

    std::filesystem::path parent(argv[0]);
    parent = parent.parent_path();
    std::filesystem::path music_path(argv[1]);

    fle::Global::visible_focus(false);
    fle::Global::visual(FL_RGB);
    fle::InitializeWin11Style();
    fle::DoubleWindow window(400_px, 400_px);
    fle::Flow main(0_px, 0_px, 400_px, 400_px);
    window.add(main);

    fle::Flow top_bar(0_px, 0_px, 1_px, 30_px);
    main.rule(top_bar, "^<=>");
    fle::RadioButton player_btn(0_px, 0_px, 1_px, 1_px);
    player_btn.label("Player");
    player_btn.labelsize(player_btn.labelsize());
    player_btn.box(fle::box(fle::WIN11_BTN_UP));
    player_btn.down_box(fle::box(fle::WIN11_BTN_DOWN));
    fle::RadioButton details_btn(0_px, 0_px, 1_px, 1_px);
    details_btn.label("Details");
    details_btn.labelsize(player_btn.labelsize());
    details_btn.box(fle::box(fle::WIN11_BTN_UP));
    details_btn.down_box(fle::box(fle::WIN11_BTN_DOWN));
    top_bar.rule(player_btn, "^<");
    top_bar.rule(details_btn, "^/<");
    top_bar.rule(player_btn, "=v=>");
    top_bar.rule(details_btn, "=v=>");

    fle::Button pause_btn(0_px, 0_px, 30_px, 30_px);
    pause_btn.labelsize(pause_btn.labelsize());
    pause_btn.box(fle::box(fle::WIN11_BTN_DOWN));
    pause_btn.down_box(fle::box(fle::WIN11_BTN_UP));
    pause_btn.type(FL_TOGGLE_BUTTON);
    main.rule(pause_btn, "<^");
    player_btn.value(1);

    fle::FillSlider vol_slider(0_px, 0_px, 30_px, 1_px);
    vol_slider.range(0, -20);
    vol_slider.box(fle::box(fle::WIN11_PROGRESS_BAR));
    vol_slider.slider(fle::box(fle::WIN11_PROGRESS_FILL));
    main.rule(vol_slider, "<=^");

    fle::Scroll panel(0_px, 0_px, 1_px, 1_px);
    panel.type(0);
    main.rule(panel, "=<=^");
    fle::Flow player(0_px, 0_px, 370_px, 370_px);
    fle::Flow details(370_px, 0_px, 370_px, 370_px);
    panel.add(player);
    panel.add(details);

    fle::HorFillSlider progress_control(0_px, 0_px, 1_px, 1_px);
    progress_control.slider(fle::box(fle::WIN11_PROGRESS_FILL));
    progress_control.box(fle::box(fle::WIN11_PROGRESS_BAR));
    progress_control.color(fle::Color(0xFDFDFD));
    player.rule(progress_control, "=^=<");

    player_btn.callback([&]() { panel.scroll_to(0, 0); });
    details_btn.callback([&]() { panel.scroll_to(370_px, 0); });

    Pa_Initialize();

    SNDSound sound{};
    SNDFILE* file = sf_wchar_open(music_path.c_str(), SFM_READ, &sound.info_);
    sound.buffer_.resize(sound.info_.channels * sound.info_.frames);
    sf_read_float(file, sound.buffer_.data(), sound.buffer_.size());
    sf_close(file);

    std::fstream config_file((parent.wstring() + L"/" + L"config.txt").c_str(), std::ios_base::in);
    if (config_file)
    {
        double vol_db = 0;
        config_file >> vol_db;
        vol_slider.value(vol_db);
    }
    else
    {
        vol_slider.value(0);
    }
    config_file.close();

    std::unique_ptr<TagLib::FileRef> music_file(new TagLib::FileRef(music_path.c_str()));
    TagLib::ByteVector music_image_data;
    auto pictures = music_file->complexProperties("PICTURE");
    if (!pictures.isEmpty())
    {
        music_image_data = pictures[0]["data"].toByteVector();
        int w = 0, h = 0, d = 0;
        unsigned char* pixels =
            stbi_load_from_memory((unsigned char*)music_image_data.data(), music_image_data.size(), //
                                  &w, &h, &d, STBI_rgb);

        fle::RGBImage::RGB_scaling(FL_RGB_SCALING_BILINEAR);
        fle::RGBImage music_image(pixels, w, h);
        down_scaled = music_image.copy(370_px, 370_px);
        down_scaled_bg = music_image.copy(370_px, 370_px);
        down_scaled_bg->desaturate();
        stbi_image_free(pixels);
        music_image_data.clear();

        Fl::set_boxtype(
            fle::box(fle::FREE_BOX),
            [](int x, int y, int w, int h, Fl_Color c)
            {
                down_scaled->draw(x, y, w, h);
                fl_rect(x + w, y, 2, h, fle::Color(0x005499));
            },
            0, 0, 0, 0);
        Fl::set_boxtype(
            fle::box(fle::FREE_BOX + 1),
            [](int x, int y, int w, int h, Fl_Color c)
            {
                down_scaled_bg->draw(x, y, w, h);
                fl_rect(x + w, y, 2, h, fle::Color(0x005499));
            },
            0, 0, 0, 0);

        progress_control.slider(fle::box(fle::FREE_BOX));
        progress_control.box(fle::box(fle::FREE_BOX + 1));
    }

    fl::TextBuffer buffer;
    buffer.append("Title: ");
    buffer.append((music_file->tag()->title().to8Bit(true).c_str()));
    buffer.append("\n");
    buffer.append("Album: ");
    buffer.append((music_file->tag()->album().to8Bit(true).c_str()));
    buffer.append("\n");
    buffer.append("Album Artist: ");
    buffer.append((music_file->properties()["ALBUMARTIST"].toString(";").to8Bit(true).c_str()));
    buffer.append("\n");
    buffer.append("Artist: ");
    buffer.append((music_file->tag()->artist().to8Bit(true).c_str()));
    buffer.append("\n");
    buffer.append("Date: ");
    buffer.append((std::to_string(music_file->tag()->year()).c_str()));
    buffer.append("\n");
    buffer.append("Genre: ");
    buffer.append(music_file->tag()->genre().to8Bit(true).c_str());
    buffer.append("\n");
    window.copy_label((music_file->tag()->title().to8Bit(true).c_str()));

    if (music_file->properties().contains("REPLAYGAIN_TRACK_GAIN"))
    {
        std::string gain_str = music_file->properties()["REPLAYGAIN_TRACK_GAIN"][0].to8Bit(true);
        gain_str = std::string(gain_str.begin(), gain_str.begin() + gain_str.rfind(' '));
        std::string peak_str = "1.0";
        sound.gain_ = std::stod(gain_str);

        if (music_file->properties().contains("REPLAYGAIN_TRACK_PEAK"))
        {
            peak_str = music_file->properties()["REPLAYGAIN_TRACK_PEAK"][0].to8Bit(true);
            sound.peak_ = std::stod(peak_str);
        }
        buffer.append("Replay Gain applied with ");
        buffer.append(gain_str.c_str());
        buffer.append(" dB. Peak at ");
        buffer.append(peak_str.c_str());
        buffer.append("\n");
    }
    else
    {
        buffer.append("Replay Gain not applied\n");
    }
    buffer.append("Sample Rate: ");
    buffer.append(std::to_string(sound.info_.samplerate).c_str());
    buffer.append("Hz\n");

    fle::TextDisplay music_details(0, 0, 1_px, 1_px);
    music_details.hide_cursor();
    music_details.scrollbar_width(0);
    music_details.selection_color(fle::Color(0x0078D4));
    music_details.wrap_mode(fle::TextDisplay::WRAP_AT_BOUNDS, 0);
    music_details.textsize(music_details.textsize());
    music_details.box(fle::box(fle::WIN11_INPUT_ACTIVE));
    music_details.buffer(buffer);
    details.rule(music_details, "=<=^");

    delete music_file.release();
    bool progress_changing = false;
    progress_control.range(0, sound.info_.frames);
    progress_control.value(0);
    progress_control.when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
    progress_control.callback(
        [&]()
        {
            if (progress_changing && !progress_control.changed())
            {
                sound.curr_p_ = progress_control.value() * sound.info_.channels;
                progress_changing = false;
            }
            else
            {
                progress_changing = true;
            }
        });

    PaStream* stream = nullptr;
    Pa_OpenDefaultStream(&stream, 0, sound.info_.channels, paFloat32, sound.info_.samplerate, 256, stream_callback,
                         &sound);
    Pa_StartStream(stream);
    pause_btn.callback(
        [&]()
        {
            if (pause_btn.value())
            {
                pause_btn.label("@>");
                Pa_StopStream(stream);
            }
            else
            {
                pause_btn.label("| |");
                Pa_StartStream(stream);
            };
        });

    window.show();
    player_btn.value(1);
    player_btn.do_callback();
    pause_btn.do_callback();
    while (Fl::check() && Fl::wait(0.008))
    {
        sound.vol_db_ = vol_slider.value();
        if (!progress_changing)
        {
            progress_control.value(sound.curr_p_ / sound.info_.channels);
        }
    }

    config_file.open((parent.wstring() + L"/" + L"config.txt").c_str(), std::ios_base::out);
    config_file << vol_slider.value();
    config_file.close();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return EXIT_SUCCESS;
}
