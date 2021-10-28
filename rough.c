Set * createSet(int capacity) {
        Set * newSet = (Set *) malloc(sizeof(Set));
        newSet->head = newSet->tail = NULL;
        newSet->occupied = 0;
        newSet->capacity = capacity;
        return newSet;
