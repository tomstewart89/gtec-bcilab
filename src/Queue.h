#ifndef QUEUE_H
#define QUEUE_H

#if defined TARGET_HAS_ThirdPartyGUSBampCAPI_Linux

#include <stdint.h>
#include <string.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

template <class QueueType> class Queue
{
    QueueType *buffer;
    int head, tail;
    int size;

public:

    Queue()	{ size = head = tail = 0; }

    Queue(QueueType *array, int len)
    {
        head = tail = 0;
        size = len;
        buffer = array;
    }

    void SetBuffer(QueueType *array, int len)
    {
        size = len;
        head = tail = 0;
        buffer = array;
    }

    int Get(QueueType *elements, int len)
    {
        // Trim the length if necessary to only as large as the number of available elements in the buffer
        len = MIN(len, Avail());

        int nonwrapped = MIN((size - tail), len), wrapped = len - nonwrapped;

        // memcpy the data starting at the head all the way up to the last element *(storage - 1)
        memcpy(elements, (buffer + tail), nonwrapped * sizeof(QueueType));

        // If there's still data to copy memcpy whatever remains, starting at the first element *(begin) until the end of data. The first step will have ensured
        // that we don't crash into the tail during this process.
        memcpy((elements + nonwrapped), buffer, wrapped * sizeof(QueueType));

        // Recalculate head
        tail = (tail + nonwrapped + wrapped) % size;

        return len;
    }

    // Returns the number of bytes actually placed in the array
    int Put(const QueueType *elements, int len)
    {
        // Trim the length if necessary to only as large as the nuber of free elements in the buffer
        len = MIN(len, Free());

        // Figure out how much to append to the end of the buffer and how much will overlap onto the start
        int nonwrapped = MIN((size - head), len), wrapped = len - nonwrapped;

        // memcpy the data starting at the head all the way up to the last element *(storage - 1)
        memcpy((buffer + head), elements, nonwrapped * sizeof(QueueType));

        // If there's still data to copy memcpy whatever remains onto the beginning of the array
        memcpy(buffer,(elements + nonwrapped), wrapped * sizeof(QueueType));

        // Re-recalculate head
        head = (head + nonwrapped + wrapped) % size;

        return len;
    }

    // Removes the oldest entry from the Queue
    void Pop() { if(Avail()) tail = (tail + 1) % size; }

    // Returns the oldest element in the array (the one added before any other)
    QueueType &Tail() { return buffer[tail]; }

    // Returns the newest element in the array (the one added after every other)
    QueueType &Head() { return buffer[(head + size - 1) % size]; }

    QueueType &operator[] (int n) { return buffer[tail + n % size]; }

    void Clear() { head = tail = 0; }

    int Avail() { return (size + head - tail) % size; }

    int Free() { return (size - 1 - Avail()); }
};

#endif // TARGET_HAS_ThirdPartyGUSBampCAPI_Linux

#endif // QUEUE_H

