namespace mrg
{
    template<class T>
    std::map<T, int> ComputeHistogram(const std::vector<T> &channel)
    {
        std::map<T, int> hist;
        for(auto pixel : channel)
        {
            if(hist.count(pixel) <= 0)
                hist[pixel] = 0;
            hist[pixel]++;
        }
        return hist;
    }

    template<class T>
    std::map<T, double> ComputeNormalizedHistogram(const std::vector<T> &channel, uint32_t width, uint32_t height)
    {
        std::map<T, int> histogram = ComputeHistogram(channel);
        uint32_t totalPixelNumber = height * width;
        std::map<T, double> normHistogram;

        for (auto it = histogram.begin(); it != histogram.end(); it++ )
        {
            double newValue = static_cast<float>(it->second) / static_cast<float>(totalPixelNumber);
            normHistogram[it->first] = newValue;
        }
        return normHistogram;
    }

    template<class ImageType, class ReturnType>
    Matrix<ReturnType> ToGrayScale(const Matrix<ImageType>& img)
    {
        const uint32_t width = img.Width();
        const uint32_t height = img.Height();
        const uint8_t channel = img.Channel();

        const auto& data = img.Data();
        std::vector<ReturnType> resultData;
        resultData.resize(width * height);

        for(unsigned int x = 0; x < width; x++)
        {
            for(unsigned int y = 0; y < height; y++)
            {
                double pixel = 0;
                for(unsigned int k = 0; k < channel; k++)
                {
                    pixel += data[(x * height + y) * channel + k];
                }
                resultData[x * height + y] = static_cast<ReturnType>(mrg::Floor(pixel / channel));
            }
        }

        return Matrix<ReturnType>(resultData, width, height, 1);
    }

    template<class Type>
    Matrix<double> Sobel(const Matrix<Type>& img)
    {
        const uint32_t width = img.Width();
        const uint32_t height = img.Height();

        Matrix<double> gray = ToGrayScale<Type, double>(img);

        const double kernelH[3][3] = {{-1., 0., 1.},
                                      {-2., 0., 2.},
                                      {-1., 0., 1.}};

        const double kernelV[3][3] = {{-1., -2., -1.},
                                      { 0.,  0.,  0.},
                                      { 1.,  2.,  1.}};

        std::vector<double> grayData = gray.Data();
        auto resultData = std::vector<double>(width * height);

        for(uint32_t i = 1; i < height - 1; i++)
        {
            for(uint32_t j = 1; j < width - 1; j++)
            {
                double magnitudeX = 0.;
                double magnitudeY = 0.;
                for(uint8_t ik = 0; ik < 3; ik++)
                {
                    for(uint8_t jk = 0; jk < 3; jk++)
                    {
                        unsigned int xn = i + ik - 1;
                        unsigned int yn = j + jk - 1;

                        unsigned int index = xn * width + yn;
                        magnitudeX += grayData[index] * kernelH[ik][jk];
                        magnitudeY += grayData[index] * kernelV[ik][jk];
                    }
                }

                resultData[i * width + j] = mrg::Sqrt(magnitudeX * magnitudeX
                                                        + magnitudeY * magnitudeY);
            }
        }

        return Matrix<double>(resultData, width, height, 1);
    }

    template<class Type>
    Matrix<double> Canny(const Matrix<Type>& img)
    {
        // Based on https://towardsdatascience.com/canny-edge-detection-step-by-step-in-python-computer-vision-b49c3a2d8123
        const uint32_t width  = img.Width();
        const uint32_t height = img.Height();

        Matrix<Type> blurred = img;

        mrg::Convolve(blurred, mrg::gaussianBlurKernel5x5);

        Matrix<double> gray = ToGrayScale<Type, double>(blurred);
        const double kernelH[3][3] = {{-1, 0, 1},
                                      {-2, 0, 2},
                                      {-1, 0, 1}};

        const double kernelV[3][3] = {{-1, -2, -1},
                                      { 0,  0,  0},
                                      { 1,  2,  1}};

        std::vector<double> grayData = gray.Data();
        auto gradientData = std::vector<double>(width * height);
        auto directionData = std::vector<double>(width * height);

        double maximumValue = 0;

        for(uint32_t i = 1; i < height - 1; i++)
        {
            for(uint32_t j = 1; j < width - 1; j++)
            {
                double magnitudeX = 0;
                double magnitudeY = 0;
                for(uint8_t ik = 0; ik < 3; ik++)
                {
                    for(uint8_t jk = 0; jk < 3; jk++)
                    {
                        unsigned int xn = i + ik - 1;
                        unsigned int yn = j + jk - 1;

                        unsigned int index = xn * width + yn;
                        magnitudeX += grayData[index] * kernelH[ik][jk];
                        magnitudeY += grayData[index] * kernelV[ik][jk];
                    }
                }

                double currentValue = mrg::Sqrt(magnitudeX * magnitudeX
                                                + magnitudeY * magnitudeY);
                gradientData[i * width + j] = currentValue;
                directionData[i * width + j] = (mrg::Atan(magnitudeX / magnitudeY) * 180.0) / mrg::Pi;
                if(directionData[i * width + j] < 0)
                    directionData[i * width + j] += 180;

                if(currentValue > maximumValue)
                    maximumValue = currentValue;
            }
        }

        // Non Maximum Suppression
        auto resultData = std::vector<double>(width * height);
        for(uint32_t i = 1; i < height - 1; i++)
        {
            for(uint32_t j = 1; j < width - 1; j++)
            {
                double q = maximumValue;
                double r = maximumValue;
                double currentAngle = directionData[i * width + j];

                // TODO: Adapt values to more than 8 bits depth
                if(0 <= currentAngle || (157.5 <= currentAngle && currentAngle <= 180))
                {
                    q = directionData[i * width + (j + 1)];
                    r = directionData[i * width + (j - 1)];
                }
                else if(22.5 <= currentAngle && currentAngle < 67.5)
                {
                    q = directionData[(i+1) * width + (j - 1)];
                    r = directionData[(i-1) * width + (j + 1)];
                }
                else if(67.5 <= currentAngle && currentAngle < 112.5)
                {
                    q = directionData[(i+1) * width + j];
                    r = directionData[(i-1) * width + j];
                }
                else if(112.5 <= currentAngle && currentAngle < 157.5)
                {
                    q = directionData[(i-1) * width + (j - 1)];
                    r = directionData[(i+1) * width + (j + 1)];
                }

                if(gradientData[i * width + j] >= q && gradientData[i * width + j] >= r)
                    resultData[i * width + j] = gradientData[i * width + j];
                else
                    resultData[i * width + j] = 0;
            }
        }

        auto resultImg = Matrix<double>(resultData, width, height, 1);
        Threshold(resultImg);

        for(uint32_t i = 1; i < height - 1; i++)
        {
            for(uint32_t j = 1; j < width - 1; j++)
            {
                if(resultData[i * width + j] == 127.0)
                {
                    double ul = resultData[(i-1) * width + (j + 1)];
                    double uu = resultData[(i) * width + (j + 1)];
                    double ur = resultData[(i+1) * width + (j + 1)];
                    double l  = resultData[(i-1) * width + (j)];
                    double r  = resultData[(i+1) * width + (j)];
                    double dl = resultData[(i-1) * width + (j - 1)];
                    double du = resultData[(i) * width + (j - 1)];
                    double dr = resultData[(i+1) * width + (j - 1)];

                    // TODO: Change 255 to image depth
                    if(ul == 255.0 || uu == 255.0 || ur == 255.0 ||
                       l == 255.0 || r == 255.0 ||
                       dl == 255.0 || du == 255.0 || dr == 255.0)
                        resultData[i * width + j] = 255.0;
                    else
                        resultData[i * width + j] = 0.0;
                }
            }
        }

        return resultImg;
    }

    template<class ImageType, class KernelType>
    void Convolve(Matrix<ImageType>& img, const Matrix<KernelType>& kernel)
    {
        const uint32_t kernelCenter = static_cast<uint32_t>(Floor(kernel.Width() / 2));
        const uint32_t width  = img.Width();
        const uint32_t height = img.Height();
        const uint8_t channel = img.Channel();

        auto& data = img.Data();
        for(uint32_t i = 0 + kernelCenter; i < height - kernelCenter; i++)
        {
            for(uint32_t j = 0 + kernelCenter; j < width - kernelCenter; j++)
            {
                for(uint8_t k = 0; k < channel; k++)
                {
                    ImageType value = ImageType(0);
                    for(uint32_t ik = 0; ik < kernel.Width(); ik++)
                    {
                        for(uint32_t jk = 0; jk < kernel.Height(); jk++)
                        {
                            uint32_t xn = i + ik - kernelCenter;
                            uint32_t yn = j + jk - kernelCenter;

                            uint32_t index = (xn * width + yn) * channel + k;
                            value += static_cast<ImageType>(mrg::Trunc(data[index] * kernel.Get(ik, jk, 0)));
                        }
                    }
                    data[(i * width + j) * channel + k] = value;
                }
            }
        }
    }

    template<class ImageType>
    void Threshold(Matrix<ImageType>& img)
    {
        [[maybe_unused]] const uint8_t channel = img.Channel();
        assert(channel == 1);

        auto& data = img.Data();

        // http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html
        //TODO: Change uint32_t to bit depth type
        std::map<ImageType, uint32_t> hist;
        for(unsigned int i = 0; i < data.size(); i++)
        {
            ImageType h = data[i];
            if(hist.count(h) <= 0)
                hist[h] = 0;
            hist[h]++;
        }

        std::size_t total = data.size();
        double sum = 0;
        for (auto it = hist.begin(); it != hist.end(); it++)
            sum += it->first * it->second;

        double sumB = 0;
        int weightBackground = 0;
        int weightForeground = 0;

        double varMax = 0;
        ImageType threshold = ImageType(0);

        for (auto it = hist.begin(); it != hist.end(); it++ )
        {
            weightBackground += static_cast<int>(it->second);
            weightForeground = static_cast<int>(total) - weightBackground;
            if(weightForeground == 0) break;

            sumB += static_cast<double>(it->first * it->second);

            double meanBackground = sumB / static_cast<double>(weightBackground);
            double meanForeground = (sum - sumB) / static_cast<double>(weightForeground);

            double varBetween = static_cast<double>(weightBackground) * static_cast<double>(weightForeground) *
                               (meanBackground - meanForeground) *
                               (meanBackground - meanForeground);

            if(varBetween > varMax)
            {
                varMax = varBetween;
                threshold = it->first;
            }
        }
        for(unsigned int i = 0; i < data.size(); i++)
        {
            // TODO: Change 256 to image depth
            data[i] = (data[i] < threshold) ? 0 : 255;
        }
    }

    template<class ImageType>
    void HistogramEqualization(Matrix<ImageType>& img)
    {
        const uint32_t width  = img.Width();
        const uint32_t height = img.Height();
        const uint8_t channel = img.Channel();
        auto& data = img.Data();

        //https://en.wikipedia.org/wiki/Histogram_equalization#Implementation
        auto channelArrays = std::vector<std::vector<ImageType>>(channel);

        for(uint8_t k = 0; k < channel; k++)
            channelArrays[k] = std::vector<ImageType>(width * height);

        for(unsigned int x = 0; x < width; x++)
        {
            for(unsigned int y = 0; y < height; y++)
            {
                for(unsigned int k = 0; k < channel; k++)
                {
                    channelArrays[k][(x * height + y)] = data[(x * height + y) * channel + k];
                }
            }
        }

        auto computeEqualization = [=](std::vector<ImageType>& result) -> void
        {
            std::map<ImageType, double> normHistogram = ComputeNormalizedHistogram(result, width, height);
            std::map<ImageType, double> cumulativeHistogram;

            // Compute cumulative histogram
            double sum = 0;
            for (auto it = normHistogram.begin(); it != normHistogram.end(); it++ )
            {
                sum += normHistogram[it->first];
                cumulativeHistogram[it->first] = sum;
            }

            // Applying equalization
            for(auto& pixel : result)
            {
                // TODO: Change 256 to image depth
                pixel = static_cast<ImageType>((256 - 1) * cumulativeHistogram[pixel]);
            }
        };

        for(uint8_t k = 0; k < channel; k++)
            computeEqualization(channelArrays[k]);


        for(unsigned int x = 0; x < width; x++)
        {
            for(unsigned int y = 0; y < height; y++)
            {
                for(unsigned int k = 0; k < channel; k++)
                {
                    data[(x * height + y) * channel + k] = channelArrays[k][(x * height + y)];
                }
            }
        }
    }

    template<uint8_t kernelSize>
    Matrix<double> GenerateGaussianKernel(const int sigma)
    {
        assert(kernelSize % 2 != 0 && "The kernel k must be odd.");

        auto k = static_cast<uint8_t>(mrg::Floor(static_cast<double>(kernelSize) / 2.0));
        std::vector<double> resultKernel { kernelSize };

        for(uint8_t i = 1; i <= kernelSize; i++)
        {
            for(uint8_t j = 1; j <= kernelSize; j++)
            {
                double f = (1.0/(2.0 * mrg::Pi * sigma * sigma));
                auto n = static_cast<double>(mrg::Pow(i - (k + 1), 2) + mrg::Pow(j - (k + 1), 2));
                auto d = static_cast<double>(2 * sigma * sigma);
                resultKernel[(i-1) * kernelSize + (j-1)] = f * mrg::Exp(- n / d);
            }
        }
        return Matrix<double>(resultKernel, kernelSize, kernelSize, 1);
    }

}