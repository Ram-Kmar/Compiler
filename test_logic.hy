int main() {
    bool a = true;
    bool b = false;
    
    if (!a) {
        return 1;
    }
    
    if (!b) {
        if (a && true) {
            if (b || true) {
                return 42;
            }
        }
    }
    return 0;
}