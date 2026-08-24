#ifndef AUDIOUTILS_H
#define AUDIOUTILS_H
#pragma once
#include <cmath>
#include <QString>
#include <QStringList>

struct NoteInfo {
    QString name;
    int octave = 0;
    double targetFrequency = 0.0;
    double centsOffset = 0.0;
};

class AudioUtils {
public:
    inline static const QStringList NOTE_NAMES = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    static double noteToFrequency(int noteIndexInOctave, int octave, double a4Tuning = 440.0) {
        int midiNote = (octave + 1) * 12 + noteIndexInOctave;
        return a4Tuning * std::pow(2.0, (midiNote - 69) / 12.0);
    }

    static NoteInfo frequencyToNote(double freqHz, double a4Tuning = 440.0) {
        NoteInfo info;
        if (freqHz <= 0.0) return info;

        double midiContinuous = 69.0 + 12.0 * std::log2(freqHz / a4Tuning);
        int nearestMidi = static_cast<int>(std::round(midiContinuous));

        int absoluteNoteIndex = nearestMidi - 12;
        if (absoluteNoteIndex < 0) absoluteNoteIndex = 0;

        info.name = NOTE_NAMES[absoluteNoteIndex % 12];
        info.octave = absoluteNoteIndex / 12;
        info.targetFrequency = a4Tuning * std::pow(2.0, (nearestMidi - 69) / 12.0);
        info.centsOffset = (midiContinuous - nearestMidi) * 100.0;

        return info;
    }

    static NoteInfo periodMsToNote(double periodMs, double a4Tuning = 440.0) {
        if (periodMs <= 0.0) return NoteInfo();
        return frequencyToNote(1000.0 / periodMs, a4Tuning);
    }
};
#endif // AUDIOUTILS_H
