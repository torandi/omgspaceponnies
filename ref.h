#ifndef REF_H
#define REF_H

template <class T>
class Ref {
	public:
		Ref(): _ref(1){};
		virtual ~Ref(){}

		T* retain(){ _ref++; return static_cast<T*>(this); }
		const T* retain() const { _ref++; return static_cast<const T*>(this); }
		void release() const {
			if ( !this ){
				return;
			}

			_ref--;

			if ( _ref == 0 ){
				delete this;
			}
		}

	private:
		mutable int _ref;
};

#endif /* REF_H */
