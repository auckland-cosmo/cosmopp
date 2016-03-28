#include <fstream>
#include <sstream>

#include <macros.hpp>
#include <exception_handler.hpp>
#include <modecode.hpp>

int main(int argc, char *argv[])
{
    try {
        StandardException exc;
        if(argc < 22)
        {
            std::string exceptionString = "Need to specify the 5 potential params.";
            exc.set(exceptionString);
            throw exc;
        }

        std::stringstream args;
        for(int i = 1; i < 22; ++i)
            args << argv[i];
        std::vector<double> v(21);
        for(int i = 0; i < 21; ++i)
            args >> v[i];

        for(int i = 1; i < 21; ++i)
            v[i] = std::pow(10.0, v[i]);
        const double NPivot = 70;

        //model 1
        //const bool slowRollEnd = true;
        //const bool eternalInflOK = false;

        //model 2
        const bool slowRollEnd = false;
        const bool eternalInflOK = true;
        ModeCode::initialize(14, 0.05, NPivot, false, false, slowRollEnd, eternalInflOK, 5e-6, 1.2, 500);

        ModeCode::addKValue(10, 0, 1e10, 0, 1e10);
        ModeCode::addKValue(1e2, 0, 1e10, 0, 1e10);
        ModeCode::addKValue(1e3, 0, 1e10, 0, 1e10);
        ModeCode::addKValue(1e4, 0, 1e10, 0, 1e10);
        ModeCode::addKValue(1e5, 0, 1e10, 0, 1e10);
        ModeCode::addKValue(1e6, 0, 1e10, 0, 1e10);
        ModeCode::addKValue(1e7, 0, 1e10, 0, 1e10);
        ModeCode::addKValue(1e8, 0, 1e10, 0, 1e10);
        ModeCode::addKValue(1e9, 0, 1e10, 0, 1e10);

        //std::vector<double>  v1{0.063324347453767582117, 0.087259287071610569519, -0.07146682502014208016, -0.012870094586320449115,    -8.6053603888697871582};

        //output_screen("Now: " << v[0] << ' ' << v[1] << ' ' << v[2] << ' ' << v[3] << ' ' << v[4] << std::endl);
        //ModeCode::calculate(v1);

        const bool res = ModeCode::calculate(v);


        if(!res)
        {
            output_screen("ModeCode failed!" << std::endl);
            return 1;
        }
        
        const Math::TableFunction<double, double>& scalarPs = ModeCode::getScalarPs();
        const Math::TableFunction<double, double>& tensorPs = ModeCode::getTensorPs();

        std::ofstream out("ps.txt");
        for(Math::TableFunction<double, double>::const_iterator it = scalarPs.begin(); it != scalarPs.end(); ++it)
        {
            const double k = it->first;
            const double s = it->second;
            check(tensorPs.find(k) != tensorPs.end(), "");
            const double t = tensorPs.evaluate(k);
            out << k << ' ' << s << ' ' << t << std::endl;
        }

        out.close();
        return 0;

    } catch (std::exception& e)
    {
        output_screen("EXCEPTION CAUGHT!!! " << std::endl << e.what() << std::endl);
        output_screen("Terminating!" << std::endl);
        return 1;
    }
    return 0;
}
