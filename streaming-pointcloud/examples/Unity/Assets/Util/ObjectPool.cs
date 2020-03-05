using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using UnityEngine;

public class ObjectPool<T>
{
    private ConcurrentBag<T> _objects;
    private Func<T> _objectGenerator;

    public ObjectPool(Func<T> objectGenerator)
    {
        if (objectGenerator == null) throw new ArgumentNullException("objectGenerator");
        _objects = new ConcurrentBag<T>();
        _objectGenerator = objectGenerator;
    }

    public T GetObject()
    {
        T item;
        if (_objects.TryTake(out item)) return item;
        return _objectGenerator();
    }

    public void PutObject(T item)
    {
        _objects.Add(item);
        Debug.Log("putObject " + _objects.Count);
    }
}