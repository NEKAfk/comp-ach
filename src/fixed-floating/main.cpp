#include<iostream>
#include"float.h"
#include"fixed.h"
#include"half.h"
#include<string>

int main(int argc, char* argv[]) {
    try {
        if (argc <= 3) {
            throw std::invalid_argument("Need more arguments\n");
        }
        std::string round = argv[2];
        if (round != "2") {
            throw std::invalid_argument("Wrong type of rounding\n");
        }
        std::string numberType = argv[1];
        char op='+';
        uint64_t n1=0, n2=0;
        if (std::string(argv[3]).find("0x") != 0) {
            throw std::invalid_argument("InvalidHexInput\n");
        }
        n1=strtoull(argv[3], NULL, 0);
        if (argc==6) {
            op = argv[4][0];
            if (std::string(argv[5]).find("0x") != 0) {
                throw std::invalid_argument("InvalidHexInput\n");
            }
            n2=strtoull(argv[5], NULL, 0);
        }
        if (numberType.size() >= 3) {
            int A = std::stoi(numberType.substr(0, numberType.find('.')));
            int B = std::stoi(numberType.substr(numberType.find('.')+1));
            Fixed f1(n1, A, B);
            Fixed f2(n2, A, B);
            switch (op)
            {
            case '+':
                f1.add(f2);
                break;
            case '-':
                f1.sub(f2);
                break;
            case '*':
                f1.mul(f2);
                break;
            case '/':
                if (n2 == 0) {
                    std::cout << "error\n";
                    return 0;
                }
                f1.div(f2);
                break;
            }
            f1.print();
        } else if (numberType == "f") {
            Float f1(n1);
            Float f2(n2);
            switch (op)
            {
            case '+':
                f1.add(f2);
                break;
            case '-':
                f1.sub(f2);
                break;
            case '*':
                f1.mul(f2);
                break;
            case '/':
                f1.div(f2);
                break;
            }
            f1.print();
        } else if (numberType == "h") {
            Half h1(n1);
            Half h2(n2);
            switch (op)
            {
            case '+':
                h1.add(h2);
                break;
            case '-':
                h1.sub(h2);
                break;
            case '*':
                h1.mul(h2);
                break;
            case '/':
                h1.div(h2);
                break;
            }
            h1.print();
        }
    } catch (std::invalid_argument& e) {
        std::cerr << e.what();
        return 1;
    }
    return 0;
}