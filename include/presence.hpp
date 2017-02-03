/*
 * Copyright 2017 deepstreamHub GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef DEEPSTREAM_PRESENCE_HPP
#define DEEPSTREAM_PRESENCE_HPP

#include <functional>
#include <memory>
#include <vector>


namespace deepstream
{
	struct Buffer;
	struct Message;

	struct Presence
	{
		typedef Buffer Name;

		typedef std::function<void(const Name&, bool)> SubscribeFn;
		typedef std::shared_ptr<SubscribeFn> SubscribeFnPtr;
		typedef std::vector<SubscribeFnPtr> SubscriberList;

		typedef std::function<bool(const Message&)> SendFn;


		explicit Presence(const SendFn&);


		SubscribeFnPtr subscribe(const SubscribeFn&);
		void subscribe(const SubscribeFnPtr&);

		void unsubscribe(const SubscribeFnPtr&);


		void notify_(const Message&);


		SendFn send_;
		SubscriberList subscribers_;
	};
}

#endif
