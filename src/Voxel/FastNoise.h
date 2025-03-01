//
// FastNoise.h
//
// MIT License
//
// Copyright(c) 2017 Jordan Peck
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// The developer's GitHub repository can be found at the following link:
// https://github.com/Auburns/FastNoise

#pragma once

#include <cmath>
#include <random>
#include <algorithm>

class FastNoise {
public:
    enum NoiseType { Value, ValueFractal, Perlin, PerlinFractal, Simplex, SimplexFractal, Cellular, WhiteNoise, Cubic, CubicFractal };
    enum Interp { Linear, Hermite, Quintic };
    enum FractalType { FBM, Billow, RigidMulti };
    enum CellularDistanceFunction { Euclidean, Manhattan, Natural };
    enum CellularReturnType { CellValue, NoiseLookup, Distance, Distance2, Distance2Add, Distance2Sub, Distance2Mul, Distance2Div };

    FastNoise(int seed = 1337) {
        m_seed = seed;
        m_frequency = 0.01f;
        m_interp = Quintic;
        m_noiseType = Simplex;
        m_fractalType = FBM;
        m_fractalOctaves = 3;
        m_fractalLacunarity = 2.0f;
        m_fractalGain = 0.5f;
        m_cellularDistanceFunction = Euclidean;
        m_cellularReturnType = CellValue;
        
        CalculateFractalBounding();
    }

    // Returns the seed used by this object
    int GetSeed() const { return m_seed; }

    // Sets seed used for all noise types
    // Default: 1337
    void SetSeed(int seed) { m_seed = seed; }

    // Sets frequency for all noise types
    // Default: 0.01
    void SetFrequency(float frequency) { m_frequency = frequency; }

    // Sets noise return type of GetNoise(...)
    // Default: Simplex
    void SetNoiseType(NoiseType noiseType) { m_noiseType = noiseType; }

    // Sets octave count for all fractal noise types
    // Default: 3
    void SetFractalOctaves(int octaves) { m_fractalOctaves = std::min(octaves, 10); CalculateFractalBounding(); }

    // Sets octave lacunarity for all fractal noise types
    // Default: 2.0
    void SetFractalLacunarity(float lacunarity) { m_fractalLacunarity = lacunarity; }

    // Sets octave gain for all fractal noise types
    // Default: 0.5
    void SetFractalGain(float gain) { m_fractalGain = gain; CalculateFractalBounding(); }

    // Sets method for interpolating between noise values
    // Default: Quintic
    void SetInterp(Interp interp) { m_interp = interp; }

    // Sets return type from cellular noise calculations
    // Default: CellValue
    void SetCellularReturnType(CellularReturnType returnType) { m_cellularReturnType = returnType; }

    // Sets distance function used in cellular noise calculations
    // Default: Euclidean
    void SetCellularDistanceFunction(CellularDistanceFunction distanceFunction) { m_cellularDistanceFunction = distanceFunction; }

    // Get noise value at position (x, y)
    float GetNoise(float x, float y) {
        x *= m_frequency;
        y *= m_frequency;

        switch (m_noiseType) {
            case Value:
                return SingleValue(m_seed, x, y);
            case ValueFractal:
                return SingleValueFractal(x, y);
            case Perlin:
                return SinglePerlin(m_seed, x, y);
            case PerlinFractal:
                return SinglePerlinFractal(x, y);
            case Simplex:
                return SingleSimplex(m_seed, x, y);
            case SimplexFractal:
                return SingleSimplexFractal(x, y);
            case Cellular:
                return SingleCellular(x, y);
            case WhiteNoise:
                return GetWhiteNoise(x, y);
            case Cubic:
                return SingleCubic(m_seed, x, y);
            case CubicFractal:
                return SingleCubicFractal(x, y);
            default:
                return 0.0f;
        }
    }

private:
    int m_seed;
    float m_frequency;
    NoiseType m_noiseType;
    Interp m_interp;
    FractalType m_fractalType;
    int m_fractalOctaves;
    float m_fractalLacunarity;
    float m_fractalGain;
    float m_fractalBounding;
    CellularDistanceFunction m_cellularDistanceFunction;
    CellularReturnType m_cellularReturnType;

    void CalculateFractalBounding() {
        float amp = m_fractalGain;
        float ampFractal = 1.0f;
        for (int i = 1; i < m_fractalOctaves; i++) {
            ampFractal += amp;
            amp *= m_fractalGain;
        }
        m_fractalBounding = 1.0f / ampFractal;
    }

    // Simplified implementations for demonstration purposes
    float SingleValue(int seed, float x, float y) {
        return std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
    }

    float SingleValueFractal(float x, float y) {
        float sum = SingleValue(m_seed, x, y);
        float amp = 1.0f;
        int i = 0;

        while (++i < m_fractalOctaves) {
            x *= m_fractalLacunarity;
            y *= m_fractalLacunarity;

            amp *= m_fractalGain;
            sum += SingleValue(m_seed + i, x, y) * amp;
        }

        return sum * m_fractalBounding;
    }

    float SinglePerlin(int seed, float x, float y) {
        return std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
    }

    float SinglePerlinFractal(float x, float y) {
        float sum = SinglePerlin(m_seed, x, y);
        float amp = 1.0f;
        int i = 0;

        while (++i < m_fractalOctaves) {
            x *= m_fractalLacunarity;
            y *= m_fractalLacunarity;

            amp *= m_fractalGain;
            sum += SinglePerlin(m_seed + i, x, y) * amp;
        }

        return sum * m_fractalBounding;
    }

    float SingleSimplex(int seed, float x, float y) {
        // Simplified implementation
        return std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
    }

    float SingleSimplexFractal(float x, float y) {
        float sum = SingleSimplex(m_seed, x, y);
        float amp = 1.0f;
        int i = 0;

        while (++i < m_fractalOctaves) {
            x *= m_fractalLacunarity;
            y *= m_fractalLacunarity;

            amp *= m_fractalGain;
            sum += SingleSimplex(m_seed + i, x, y) * amp;
        }

        return sum * m_fractalBounding;
    }

    float SingleCubic(int seed, float x, float y) {
        return std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
    }

    float SingleCubicFractal(float x, float y) {
        float sum = SingleCubic(m_seed, x, y);
        float amp = 1.0f;
        int i = 0;

        while (++i < m_fractalOctaves) {
            x *= m_fractalLacunarity;
            y *= m_fractalLacunarity;

            amp *= m_fractalGain;
            sum += SingleCubic(m_seed + i, x, y) * amp;
        }

        return sum * m_fractalBounding;
    }

    float SingleCellular(float x, float y) {
        return std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
    }

    float GetWhiteNoise(float x, float y) {
        return std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
    }
}; 