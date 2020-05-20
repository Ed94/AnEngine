/*
Casting

Last Modified: 5/18/2020
*/



#pragma once



/*
Constant Cast
*/
template<typename Type>
Type CVCast(const Type _obj)
{
	return const_cast<Type>(_obj);
}

/*
Dynamic Cast (Direct)
*/
template<typename Derived, typename Base>
Derived DCast(const Base _obj)
{
	return dynamic_cast<Derived>(_obj);
}

/*
Dynamic Cast (Pointer)
*/
template<typename Derived, typename Base>
Derived* DCast(Base* const _ptr)
{
	return dynamic_cast< Derived* >(_ptr);
}

/*
Static Cast (Direct)
*/
template<typename Derived, typename Base>
Derived SCast(const Base _obj)
{
	return static_cast<Derived>(_obj);
}

/*
Static Cast (Pointer)
*/
template<typename Derived, typename Base>
Derived SCast(const Base* _ptr)
{
	return static_cast< Derived* >(_ptr);
}