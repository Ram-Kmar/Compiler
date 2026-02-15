layer Linear {
    int weights = 1;
    int bias = 0;
    int forward(int x) {
        return x * weights + bias;
    }
}

layer ReLU {
    int forward(int x) {
        if (x > 0) { return x; }
        else { return 0; }
    }
}

int main() {
    Linear l1;
    ReLU r1;
    Linear l2;
    
    int x = 10;
    int y = l1.forward(x);
    int z = r1.forward(y);
    int out = l2.forward(z);
    
    return out;
}
