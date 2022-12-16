class Monah {
private:
    uint64_t ci_;

    bool isCiCorrect(int ci) {
        if (ci > 1 && ci < INT_MAX) {
            return true;
        }

        return false;
    }

public:
    explicit Monah(int ci) {
        if (isCiCorrect(ci))
            ci_ = ci;
        else
            ci = 0;
    }

    int GetCi() const {
        return ci_;
    }

    void InvreaseCi2Times() {
        ci_ *= 2;
    }

    std::string ToString() const {
        std::string s = "Monah with ci = ";
        s += std::to_string(ci_);
        return s;
    }
};

std::ostream& operator << (std::ostream& os, Monah monah) {
    os << monah.ToString();
    return os;
}

bool operator == (Monah first, Monah second) {
    return first.GetCi() == second.GetCi();
}