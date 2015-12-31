

__kernel void histogram(__global const unsigned int* image, __global unsigned int* results, const unsigned int size) {
    unsigned int i = get_global_id(0) * 3;
    unsigned int index;
    if (i < size) {   
        index = image[i];
        atomic_add(&(results[index]), 1);
        
        index = image[++i];
        results += 256;
        atomic_add(&(results[index]), 1);
        
        index = image[++i];
        results += 256;
        atomic_add(&(results[index]), 1);
    }    
}
