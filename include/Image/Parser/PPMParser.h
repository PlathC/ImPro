//
// Created by Cyprien Plateau--Holleville on 22/06/2019.
//

#ifndef IMPRO_PPMPARSER_H
#define IMPRO_PPMPARSER_H

#include "IParser.h"
#include "../Matrix.h"

namespace imp {
    namespace ImageParser{
        //http://netpbm.sourceforge.net/doc/libppm.html

        template<typename T>
        class PPMParser : public IParser<T> {
        public:
            Matrix<T> Parse(std::string fileName, const unsigned int channel) override;
            void Write(Matrix<T>& mat, std::string fileName) override;
        };
    }
}

#include "PPMParser.inl"

#endif //IMPRO_PPMPARSER_H
