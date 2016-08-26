#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#include <fantom/algorithm.hpp>
#include <fantom/fields.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>

#include <string>
#include "lib/conrec_cxx.hpp"

using namespace std;
using namespace fantom;
namespace
{
class IsolinesAlgorithm : public VisAlgorithm
{
    unique_ptr< Primitive > isolines;
public:
    struct Options : public VisAlgorithm::Options {
//    public:
        Options(Options::Control &control) : VisAlgorithm::Options(control) {
            add<TensorFieldInterpolated<2, Scalar>>("Field", "interpolated TensorField");    /// interpolierte Werte an den Grid-Punkten
            add<string>("Levels", "Number os Isoline-levels", "0");
        }
    };

    struct VisOutputs : public VisAlgorithm::VisOutputs {
        VisOutputs(VisOutputs::Control &control) : VisAlgorithm::VisOutputs( control ) {
            addGraphics( "Isolines" );
        }
    };

    IsolinesAlgorithm(InitData &data) : VisAlgorithm(data) {}

    virtual void execute(const Algorithm::Options &options, const volatile bool &abortFlag) override {
        isolines = getGraphics( "Isolines" ).makePrimitive();

        shared_ptr< const TensorFieldInterpolated<2, Scalar> > field = options.get< TensorFieldInterpolated<2, Scalar> >("Field");
        if( !field ) {
            debugLog() << "Input Field not set." << endl;
            return;
        }

        shared_ptr<const Grid<2> > grid = dynamic_pointer_cast<const Grid<2> >(field->domain());
        if( !grid ) {
            throw std::logic_error( "Wrong type of grid!" );
        }

        debugLog() << "cells: " << grid->numCells() << endl;
        debugLog() << "values: " << field->values().size() << endl;
        const size_t nx = grid->structuringDimensionExtent(0);
        const size_t ny = grid->structuringDimensionExtent(1);
        debugLog() << "nx: " << nx << endl;
        debugLog() << "ny: " << ny << endl;

        /// Datenstrukturen für conrec #########################################
        double **d = new double*[ny];
        for (size_t i = 0; i < ny; ++i)
            d[i] = new double[nx];

        for (size_t y = 0; y < ny; ++y)
            for (size_t x = 0; x < nx; ++x)
                d[y][x] = field->values()[y * nx + x][0];

        double *x = new double[nx];         //x - jub
        for (size_t i = 0; i < nx; ++i)
            x[i] = grid->points()[i][0];

        double *y = new double[ny];         //y - iub
        for (size_t i = 0; i < ny; ++i)
            y[i] = grid->points()[i * nx][1];

        const size_t levels = 2;
        double z[levels] = {5, 10};
        debugLog() << "structs-init: fertig" << endl;

        /// Lines ##############################################################
        vector<Vector3> v;

        conrec(d, 0, static_cast<int>(ny), 0, static_cast<int>(nx), y, x, levels, z, v);
        debugLog() << "conrec: fertig" << endl;

        isolines->add(Primitive::LINES)
        .setLineWidth(2)
        .setColor(Color(1, 0, 0))
        .setVertices(v);
        // Aufräumen -----------------------------------------------------------
        for (size_t i = 0; i < ny; ++i) {
            delete[] d[i];
            d[i] = 0;
        }
        d = 0;
        delete[] x;
        x = 0;
        delete[] y;
        y = 0;
    }
};

AlgorithmRegister<IsolinesAlgorithm> dummy("Vis Project/Isolines", "Interpolate in a data set.");

} // namespace
